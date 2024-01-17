//
// Created by kj16609 on 12/30/23.
//

#include "Buffer.hpp"

#include "BufferSuballocationHandle.hpp"
#include "engine/Device.hpp"

namespace fgl::engine
{
	std::unique_ptr< Buffer > global_staging_buffer { nullptr };

	void initGlobalStagingBuffer( std::uint64_t size )
	{
		using namespace fgl::literals::size_literals;
		global_staging_buffer = std::make_unique< Buffer >(
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eDeviceLocal );
	}

	Buffer& getGlobalStagingBuffer()
	{
		assert( global_staging_buffer && "Global staging buffer not initialized" );
		return *global_staging_buffer.get();
	}

	BufferHandle::BufferHandle(
		vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties ) :
	  m_memory_size( memory_size ),
	  m_usage( usage ),
	  m_memory_properties( memory_properties )
	{
		alloc( memory_size );
	}

	BufferHandle::~BufferHandle()
	{
		dealloc();
	}

	void BufferHandle::dealloc()
	{
		vmaDestroyBuffer( Device::getInstance().allocator(), m_buffer, m_allocation );
	}

	void BufferHandle::alloc( vk::DeviceSize memory_size )
	{
		m_memory_size = memory_size;
		vk::BufferCreateInfo buffer_info {};
		buffer_info.pNext = VK_NULL_HANDLE;
		buffer_info.flags = {};
		buffer_info.size = m_memory_size;
		buffer_info.usage = m_usage;
		buffer_info.sharingMode = vk::SharingMode::eExclusive;
		buffer_info.queueFamilyIndexCount = 0;
		buffer_info.pQueueFamilyIndices = VK_NULL_HANDLE;

		VmaAllocationCreateInfo alloc_info {};

		alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

		if ( m_memory_properties & vk::MemoryPropertyFlagBits::eHostVisible )
			alloc_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

		if ( m_usage & vk::BufferUsageFlagBits::eTransferSrc )
		{
			//Remove VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM BIT if we are transfer src
			alloc_info.flags &= ~VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

			alloc_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}

		const VkBufferCreateInfo& vk_buffer_info = buffer_info;
		VkBuffer buffer { VK_NULL_HANDLE };
		if ( vmaCreateBuffer(
				 Device::getInstance().allocator(), &vk_buffer_info, &alloc_info, &buffer, &m_allocation, nullptr )
		     != VK_SUCCESS )
		{
			throw std::runtime_error( "Failed to allocate" );
		}

		m_buffer = buffer;

		vmaGetAllocationInfo( Device::getInstance().allocator(), m_allocation, &m_alloc_info );
	}

	std::vector< std::weak_ptr< BufferHandle > > Buffer::getActiveBufferHandles()
	{
		std::vector< std::weak_ptr< BufferHandle > > handles;
		handles.reserve( m_buffer_handles.size() );

		for ( auto& handle : m_buffer_handles )
		{
			if ( auto ptr = handle.lock() )
			{
				handles.push_back( ptr );
			}
		}

		m_buffer_handles = handles;
		return handles;
	}

	vk::DeviceSize Buffer::alignment()
	{
		vk::DeviceSize size { 0 };

		if ( m_usage & vk::BufferUsageFlagBits::eUniformBuffer )
		{
			size = std::max( size, Device::getInstance().m_properties.limits.minUniformBufferOffsetAlignment );
		}

		if ( m_memory_properties & vk::MemoryPropertyFlagBits::eHostVisible )
		{
			size = std::max( size, Device::getInstance().m_properties.limits.nonCoherentAtomSize );
		}

		return size;
	}

	std::shared_ptr< BufferSuballocationHandle > Buffer::
		suballocate( vk::DeviceSize memory_size, std::uint32_t allignment )
	{
		ZoneScoped;
		//Calculate alignment from alignment, ubo_alignment, and atom_size_alignment
		memory_size = align( memory_size, alignment() );

		//Find a free space.
		auto itter = std::find_if(
			m_free_blocks.begin(),
			m_free_blocks.end(),
			[ this, memory_size, allignment ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
			{
				auto [ offset, size ] = pair;

				const auto new_offset = align( offset, alignment(), allignment );
				size -= new_offset - offset;

				return size >= memory_size;
			} );

		if ( itter == m_free_blocks.end() )
		{
			std::cout << "======    !!! OOM !!!   ======\n"
						 "====== Allocated Blocks ======\n";

			for ( auto [ offset, size ] : m_free_blocks )
			{
				std::cout << "Offset: " << std::hex << offset << " Size: " << std::dec << size << "\n";

				std::cout << "Aligned offset: " << std::hex << align( offset, alignment(), allignment )
						  << " Size: " << std::dec << size << "\n"
						  << std::endl;
			}

			std::cout << "====== Suballocations ======\n";

			for ( auto [ offset, size ] : m_suballocations )
			{
				std::cout << "Offset: " << std::hex << offset << " Size: " << std::dec << size << "\n";
			}

			std::cout << "=============================\n"
					  << "Attempted to allocate block of size: "
					  << fgl::literals::size_literals::to_string( memory_size ) << std::endl;

			throw std::runtime_error( "Failed to find free space" );
		}

		//Allocate
		auto [ offset, size ] = *itter;
		m_free_blocks.erase( itter );

		const auto aligned_offset { align( offset, alignment(), allignment ) };

		if ( aligned_offset != offset )
		{
			m_free_blocks.emplace_back( std::make_pair( offset, aligned_offset - offset ) );
			offset = aligned_offset;
			size -= aligned_offset - offset;
		}

		m_suballocations.insert_or_assign( offset, memory_size );

		if ( size - memory_size > 0 )
			m_free_blocks.emplace_back( std::make_pair( offset + memory_size, size - memory_size ) );

		return std::make_shared< BufferSuballocationHandle >( *this, offset, memory_size );
	}

	void Buffer::free( fgl::engine::BufferSuballocationHandle& info )
	{
		ZoneScoped;

		{
			//Find the suballocation
			auto itter = m_suballocations.find( info.m_offset );

			if ( itter == m_suballocations.end() ) throw std::runtime_error( "Failed to find suballocation" );

			//Remove the suballocation
			m_suballocations.erase( itter );
		}

		// Forward check
		{
			ZoneScopedN( "Forward check" );
			auto itter = std::find_if(
				m_free_blocks.begin(),
				m_free_blocks.end(),
				[ &info ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
				{
					auto& [ offset, size ] = pair;
					return offset > info.m_offset && offset == info.m_offset + info.m_size;
				} );

			//If itter is not end then we have found a block where itter->offset > offset

			if ( itter != m_free_blocks.end() )
			{
				auto& [ free_offset, free_size ] = *itter;
				info.m_size += free_size; // Add their size to ours

				//Nuke block
				m_free_blocks.erase( itter );
			}
		}

		// Backwards check
		{
			ZoneScopedN( "Backwards check" );
			auto prev_block = std::find_if(
				m_free_blocks.begin(),
				m_free_blocks.end(),
				[ &info ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
				{
					auto& [ offset, size ] = pair;
					return offset + size + 1 == info.m_offset;
				} );

			if ( prev_block != m_free_blocks.end() )
			{
				auto& [ offset, size ] = *prev_block;
				size += info.m_size;
			}
			else
			{
				//No block before us. We are the free block
				m_free_blocks.push_back( { info.m_offset, info.m_size } );
			}
		}
	}

	void* Buffer::map( BufferSuballocationHandle& handle )
	{
		if ( m_handle->m_alloc_info.pMappedData == nullptr ) return nullptr;

		return static_cast< std::byte* >( m_handle->m_alloc_info.pMappedData ) + handle.m_offset;
	}

	Buffer::~Buffer()
	{}

	Buffer::
		Buffer( vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties ) :
	  m_handle( std::make_shared< BufferHandle >( memory_size, usage, memory_properties ) ),
	  m_usage( usage ),
	  m_memory_properties( memory_properties )
	{
		m_free_blocks.insert( m_free_blocks.begin(), { 0, memory_size } );
		m_buffer_handles.emplace_back( m_handle );
	}

} // namespace fgl::engine
