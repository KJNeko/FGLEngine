//
// Created by kj16609 on 12/30/23.
//

#include "Buffer.hpp"

#include <iostream>

#include "BufferSuballocationHandle.hpp"
#include "align.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/memory/buffers/exceptions.hpp"
#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine::memory
{

	inline static std::vector< Buffer* > active_buffers {};

	std::vector< Buffer* > getActiveBuffers()
	{
		return active_buffers;
	}

	Buffer::Buffer(
		vk::DeviceSize memory_size,
		const vk::BufferUsageFlags usage,
		const vk::MemoryPropertyFlags memory_properties ) :
	  m_memory_size( memory_size ),
	  m_usage( usage ),
	  m_memory_properties( memory_properties )
	{
		alloc( memory_size );
		active_buffers.emplace_back( this );
		m_free_blocks.emplace_back( 0, memory_size );
	}

	Buffer::~Buffer()
	{
		if ( !m_allocations.empty() )
		{
			log::critical( "Buffer allocations not empty. {} allocations left", m_allocations.size() );

			for ( const auto& [ offset, size ] : m_allocations )
			{
				log::info( "Stacktrace: Offset at {} with a size of {}", offset, size );

				const auto itter = this->m_allocation_traces.find( offset );

				if ( itter == this->m_allocation_traces.end() ) continue;

				std::stacktrace trace { itter->second };

				std::cout << trace << std::endl;
			}

			log::critical( "Buffer allocations were not empty!" );
			// Call will always terminate
			// throw std::runtime_error( "Buffer allocations not empty" );
		}

		dealloc();

		if ( const auto itter = std::ranges::find( active_buffers, this ); itter != active_buffers.end() )
			active_buffers.erase( itter );
	}

	void* Buffer::map( const BufferSuballocationHandle& handle ) const
	{
		if ( m_alloc_info.pMappedData == nullptr ) return nullptr;

		return static_cast< std::byte* >( m_alloc_info.pMappedData ) + handle.m_offset;
	}

	void Buffer::dealloc() const
	{
		vmaDestroyBuffer( Device::getInstance().allocator(), m_buffer, m_allocation );
	}

	void Buffer::alloc( const vk::DeviceSize memory_size )
	{
		assert( !m_debug_name.empty() );
		assert( memory_size > 0 );
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
			throw BufferException( "Unable to allocate memory in VMA" );
		}

		m_buffer = buffer;

		vmaGetAllocationInfo( Device::getInstance().allocator(), m_allocation, &m_alloc_info );
	}

	vk::DeviceSize Buffer::alignment() const
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

	decltype( Buffer::m_free_blocks )::iterator Buffer::
		findAvailableBlock( vk::DeviceSize memory_size, std::uint32_t t_alignment )
	{
		//Find a free space.
		return std::ranges::find_if(
			m_free_blocks,
			[ this, memory_size, t_alignment ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
			{
				const auto [ offset, size ] = pair;

				const auto new_offset = align( offset, alignment(), t_alignment );
				const auto after_size { size - ( new_offset - offset ) };

				// If the size of the block after alignment is greater than or equal to the size of the memory we want to allocate using it.
				return after_size >= memory_size;
			} );
	}

	std::shared_ptr< BufferSuballocationHandle > Buffer::
		allocate( vk::DeviceSize desired_memory_size, const std::uint32_t t_alignment )
	{
		ZoneScoped;
		//Calculate alignment from alignment, ubo_alignment, and atom_size_alignment
		desired_memory_size = align( desired_memory_size, alignment() );

		assert( desired_memory_size <= this->size() );

		//findAvailableBlock( memory_size, t_alignment );

		if ( !canAllocate( desired_memory_size, t_alignment ) )
		{
			//TODO: Write more detailed error message
			throw BufferOOM();
		}

		auto itter { findAvailableBlock( desired_memory_size, t_alignment ) };

		if ( itter == m_free_blocks.end() )
		{
			//If we can't find a block, then we need to merge the free blocks and try again
			mergeFreeBlocks();
			itter = findAvailableBlock( desired_memory_size, t_alignment );
		}

		//TODO: Move this error stuff into the exception message
		if ( itter == m_free_blocks.end() )
		{
			throw BufferOOM();
		}

		//Allocate
		auto [ selected_block_offset, selected_block_size ] = *itter;
		m_free_blocks.erase( itter );

		assert( selected_block_offset <= this->size() );
		assert( selected_block_size <= this->size() );

		const auto aligned_offset { align( selected_block_offset, alignment(), t_alignment ) };

		//Fix the offset and size if they aren't alligned
		if ( aligned_offset != selected_block_offset )
		{
			//Insert the space left over before the block starts back into the free blocks
			const std::size_t leftover_start_size { aligned_offset - selected_block_offset };

			m_free_blocks.emplace_back( std::make_pair( selected_block_offset, leftover_start_size ) );

			mergeFreeBlocks();

			selected_block_offset = aligned_offset;
			assert( selected_block_size >= leftover_start_size );
			selected_block_size -= leftover_start_size;
		}

		//Add the suballocation
		m_allocations.insert_or_assign( selected_block_offset, desired_memory_size );

		m_allocation_traces.insert_or_assign( selected_block_offset, std::stacktrace::current() );

		assert( selected_block_size >= desired_memory_size );

		assert( selected_block_size <= this->size() );

		//If there is any memory left over, Then add it back into the free blocks
		if ( ( selected_block_size >= desired_memory_size ) && ( selected_block_size - desired_memory_size > 0 ) )
		{
			assert( selected_block_size - desired_memory_size <= this->size() );
			m_free_blocks
				.emplace_back( selected_block_offset + desired_memory_size, selected_block_size - desired_memory_size );
		}

#ifndef NDEBUG
		//Check that we haven't lost any memory
		std::size_t sum { 0 };
		for ( const auto& [ _, free_size ] : this->m_free_blocks )
		{
			sum += free_size;
		}

		for ( const auto& [ _, allocated_size ] : this->m_allocations )
		{
			sum += allocated_size;
		}

		assert( sum == this->size() );
#endif

		return std::make_shared< BufferSuballocationHandle >( *this, selected_block_offset, desired_memory_size );
	}

	bool Buffer::canAllocate( const vk::DeviceSize memory_size, const std::uint32_t alignment )
	{
		// TODO: This check can be optimized by itterating through and virtually combining blocks that would be combined.
		// If the combined block is large enough then we should consider it being capable of allocation.
		// We don't need to care if a block later in the chain is large enough since the allocation would first
		// check blocks that are already large enough before trying to combine them.
		if ( findAvailableBlock( memory_size, alignment ) == m_free_blocks.end() )
		{
			mergeFreeBlocks();
			return findAvailableBlock( memory_size, alignment ) != m_free_blocks.end();
		}
		else
			return true;
	}

	void Buffer::mergeFreeBlocks()
	{
		ZoneScoped;
		//Can't combine blocks if there is only 1
		if ( m_free_blocks.size() <= 1 ) return;

		//Sort the blocks by offset
		std::ranges::
			sort( m_free_blocks, []( const auto& a, const auto& b ) noexcept -> bool { return a.first < b.first; } );

		auto itter { m_free_blocks.begin() };
		auto next_block { std::next( itter ) };

		// Search through all free blocks
		while ( next_block != m_free_blocks.end() && itter != m_free_blocks.end() )
		{
			//Is the next block adjacent to the current block?
			auto& [ offset, size ] = *itter;
			const auto& [ next_offset, next_size ] = *next_block;

			// Is the next block ajacent to us?
			const bool is_adjacent { offset + size == next_offset };

			if ( is_adjacent )
			{
				//Combine the blocks
				size += next_size;
				//Remove the next block
				m_free_blocks.erase( next_block );

				//Reset the next block
				next_block = std::next( itter );
				continue;
			}
			else
			{
				//Move to the next block
				itter = next_block;
				next_block = std::next( itter );
				continue;
			}
		}
	}

	void Buffer::setDebugName( const std::string& str )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eBuffer;
		info.pObjectName = str.c_str();
		info.objectHandle = reinterpret_cast< std::uint64_t >( static_cast< VkBuffer >( this->m_buffer ) );

		m_debug_name = str;

		Device::getInstance().setDebugUtilsObjectName( info );
	}

	void Buffer::free( BufferSuballocationHandle& info )
	{
		ZoneScoped;

		//Find the suballocation
		auto itter = m_allocations.find( info.m_offset );

		if ( itter == m_allocations.end() ) throw std::runtime_error( "Failed to find suballocation" );

		//Remove the suballocation
		m_allocations.erase( itter );

		if ( info.m_offset >= this->size() ) throw std::runtime_error( "Offset was outside of bounds of buffer" );
		if ( info.m_offset + info.m_size > this->size() )
			throw std::runtime_error(
				std::format(
					"m_offset + m_size was outside the bounds of the buffer ({} + {} == {} >= {})",
					info.m_offset,
					info.m_size,
					info.m_offset + info.m_size,
					size() ) );

		//Add the block back to the free blocks
		m_free_blocks.emplace_back( info.m_offset, info.m_size );

		mergeFreeBlocks();

#ifndef NDEBUG
		//Check that we haven't lost any memory
		std::size_t sum { 0 };
		for ( const auto& free_blocks : this->m_free_blocks )
		{
			sum += free_blocks.second;
		}

		for ( const auto& allocated : this->m_allocations )
		{
			sum += allocated.second;
		}

		if ( sum != this->size() )
			throw std::runtime_error( std::format( "Memory leaked! Expected {} was {}", this->size(), sum ) );
#endif
	}

	vk::DeviceSize Buffer::used() const
	{
		vk::DeviceSize total_size { 0 };

		if ( m_allocations.size() == 0 ) return total_size;

		for ( const auto& [ offset, size ] : m_allocations )
		{
			total_size += size;
		}

		return total_size;
	}

	vk::DeviceSize Buffer::largestBlock() const
	{
		vk::DeviceSize largest { 0 };

		for ( const auto& blocks : m_free_blocks )
		{
			largest = std::max( largest, blocks.second );
		}

		return largest;
	}

} // namespace fgl::engine::memory
