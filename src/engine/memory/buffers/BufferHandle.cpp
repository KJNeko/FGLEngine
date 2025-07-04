//
// Created by kj16609 on 12/30/23.
//

#include "BufferHandle.hpp"

#include <iostream>
#include <tuple>

#include "BufferSuballocationHandle.hpp"
#include "align.hpp"
#include "assets/transfer/TransferManager.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/memory/buffers/BufferHandle.hpp"
#include "engine/memory/buffers/exceptions.hpp"
#include "engine/rendering/devices/Device.hpp"
#include "math/literals/size.hpp"
#include "memory/DefferedCleanup.hpp"

namespace fgl::engine::memory
{

	void BufferHandle::swap( BufferHandle& other ) noexcept
	{
		std::swap( m_buffer, other.m_buffer );
		std::swap( m_allocation, other.m_allocation );
		std::swap( m_alloc_info, other.m_alloc_info );
		std::swap( m_memory_size, other.m_memory_size );
		std::swap( m_usage, other.m_usage );
		std::swap( m_memory_properties, other.m_memory_properties );
		std::swap( m_track, other.m_track );
		std::swap( m_debug_name, other.m_debug_name );
		std::swap( m_active_suballocations, other.m_active_suballocations );
		std::swap( m_allocation_traces, other.m_allocation_traces );
		std::swap( m_free_blocks, other.m_free_blocks );
	}

	BufferHandle::BufferHandle(
		vk::DeviceSize memory_size,
		const vk::BufferUsageFlags usage,
		const vk::MemoryPropertyFlags memory_properties ) :
	  m_memory_size( memory_size ),
	  m_usage( usage ),
	  m_memory_properties( memory_properties )
	{
		auto [ buffer, vma_alloc_info, vma_allocation ] = allocBuffer( memory_size, m_usage, m_memory_properties );

		m_buffer = buffer;
		m_alloc_info = vma_alloc_info;
		m_allocation = vma_allocation;

		m_free_blocks.emplace_back( 0, memory_size );
	}

	BufferHandle::~BufferHandle()
	{
		if ( !m_active_suballocations.empty() )
		{
			log::critical( "Buffer allocations not empty. {} allocations left", m_active_suballocations.size() );

			for ( const auto& suballocation : m_active_suballocations )
			{
				if ( suballocation.expired() ) continue;
				auto suballoc_ptr { suballocation.lock() };
				const auto offset { suballoc_ptr->offset() };
				const auto size { suballoc_ptr->size() };
				log::info( "Stacktrace: Offset at {} with a size of {}", offset, size );

				const auto itter = this->m_allocation_traces.find( offset );

				if ( itter == this->m_allocation_traces.end() ) continue;

				std::stacktrace trace { itter->second };

				std::cout << trace << std::endl;
			}

			log::critical( "Buffer allocations were not empty!" );
			// Call will always terminate
			throw std::runtime_error( "Buffer allocations not empty" );
		}

		deallocBuffer( m_buffer, m_allocation );
	}

	void* BufferHandle::map( const BufferSuballocationHandle& handle ) const
	{
		if ( m_alloc_info.pMappedData == nullptr ) return nullptr;

		return static_cast< std::byte* >( m_alloc_info.pMappedData ) + handle.offset();
	}

	void BufferHandle::deallocBuffer( const vk::Buffer& buffer, const VmaAllocation& allocation )
	{
		vmaDestroyBuffer( Device::getInstance().allocator(), buffer, allocation );
	}

	std::tuple< vk::Buffer, VmaAllocationInfo, VmaAllocation > BufferHandle::allocBuffer(
		const vk::DeviceSize memory_size, vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags property_flags )
	{
		// Used for resizing.
		//TODO: Make this only available if resize is desired. Otherwise do not have it.
		usage |= vk::BufferUsageFlagBits::eTransferDst;
		usage |= vk::BufferUsageFlagBits::eTransferSrc;

		assert( memory_size > 0 );
		vk::BufferCreateInfo buffer_info {};
		buffer_info.pNext = VK_NULL_HANDLE;
		buffer_info.flags = {};
		buffer_info.size = memory_size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = vk::SharingMode::eExclusive;
		buffer_info.queueFamilyIndexCount = 0;
		buffer_info.pQueueFamilyIndices = VK_NULL_HANDLE;

		VmaAllocationCreateInfo create_info {};

		create_info.usage = VMA_MEMORY_USAGE_AUTO;

		if ( property_flags & vk::MemoryPropertyFlagBits::eHostVisible )
			create_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

		if ( usage & vk::BufferUsageFlagBits::eTransferSrc )
		{
			//Remove VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM BIT if we are transfer src
			create_info.flags &= ~VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

			create_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}

		VmaAllocationInfo alloc_info {};
		VmaAllocation allocation {};

		const VkBufferCreateInfo& vk_buffer_info = buffer_info;
		VkBuffer buffer { VK_NULL_HANDLE };
		if ( vmaCreateBuffer(
				 Device::getInstance().allocator(), &vk_buffer_info, &create_info, &buffer, &allocation, nullptr )
		     != VK_SUCCESS )
		{
			throw BufferException( "Unable to allocate memory in VMA" );
		}

		vmaGetAllocationInfo( Device::getInstance().allocator(), allocation, &alloc_info );

		return std::make_tuple<
			vk::Buffer,
			VmaAllocationInfo,
			VmaAllocation >( std::move( buffer ), std::move( alloc_info ), std::move( allocation ) );
	}

	vk::DeviceSize BufferHandle::alignment() const
	{
		vk::DeviceSize size { 1 };

		if ( m_usage & vk::BufferUsageFlagBits::eStorageBuffer )
		{
			size = std::max( size, Device::getInstance().m_properties.limits.minStorageBufferOffsetAlignment );
		}

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

	decltype( BufferHandle::m_free_blocks )::iterator BufferHandle::
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

	BufferSuballocation Buffer::allocate( const vk::DeviceSize desired_size, const std::uint32_t alignment )
	{
		auto allocation { operator->()->allocate( desired_size, alignment ) };

		if ( !allocation )
		{
			// Resize to x1.5 the size, or the size plus the desired size x1.5, Whichever is bigger
			const auto optimal_size { std::max( this->size() * 2, this->size() + desired_size * 2 ) };
			this->resize( optimal_size );

			FGL_ASSERT( optimal_size == this->size(), "Optimal size not met!" );

			allocation = operator->()->allocate( desired_size, alignment );
		}

		return allocation;
	}

	void Buffer::resize( const vk::DeviceSize size )
	{
		const std::shared_ptr< BufferHandle > old_handle { *this };
		const auto new_handle { old_handle->remake( size ) };
		new_handle->setDebugName( old_handle->m_debug_name );

		*this = new_handle;
	}

	std::shared_ptr< BufferHandle > BufferHandle::remake( vk::DeviceSize new_size )
	{
		ZoneScoped;
		auto new_handle { std::make_shared< BufferHandle >( new_size, m_usage, m_memory_properties ) };

		const auto& old_allocations { m_active_suballocations };
		const auto& old_allocations_traces { m_allocation_traces };

		// Remake all allocations into the new buffer
		std::vector<
			std::pair< std::shared_ptr< BufferSuballocationHandle >, std::shared_ptr< BufferSuballocationHandle > > >
			allocations {};
		allocations.reserve( old_allocations.size() );

		for ( const auto& suballocation_weak : old_allocations )
		{
			if ( suballocation_weak.expired() ) continue;

			auto suballocation { suballocation_weak.lock() };

			auto [ old_suballocation, new_suballocation ] = suballocation->reallocate( new_handle );

			allocations.emplace_back( old_suballocation, new_suballocation );

			// Copy the data from the old allocation to the new allocation
			TransferManager::getInstance().copySuballocationRegion( old_suballocation, new_suballocation );

			old_suballocation->flagReallocated( new_suballocation );
		}

		return new_handle;
	}

	/*
	void BufferHandle::resize( const vk::DeviceSize new_size )
	{
		log::warn( "Resizing buffer from {} to {}", size(), new_size );

		std::shared_ptr< BufferHandle > new_handle { new BufferHandle( new_size, m_usage, m_memory_properties ) };

		//Now we need to re-create all the current live allocations and transfer/replace them using the new buffer
		std::vector<
			std::pair< std::shared_ptr< BufferSuballocationHandle >, std::shared_ptr< BufferSuballocationHandle > > >
			allocations {};

		allocations.reserve( m_active_suballocations.size() );

		for ( auto& suballocation_weak : m_active_suballocations )
		{
			if ( suballocation_weak.expired() ) continue;
			try
			{
				auto suballocation { suballocation_weak.lock() };

				auto new_suballocation { new_handle->allocate( suballocation->m_size, suballocation->m_alignment ) };
				allocations.emplace_back( suballocation, new_suballocation );
			}
			catch ( std::bad_weak_ptr& e )
			{
				// noop
				void();
			}
		}

		std::swap( m_buffer, new_handle->m_buffer );
		std::swap( m_alloc_info, new_handle->m_alloc_info );
		std::swap( m_allocation, new_handle->m_allocation );
		std::swap( m_free_blocks, new_handle->m_free_blocks );
		std::swap( m_active_suballocations, new_handle->m_active_suballocations );
		std::swap( m_allocation_traces, new_handle->m_allocation_traces );

		// This transforms any memory::Buffer to be the `new_handle` we allocated above.
		const auto old_handle { new_handle };
		FGL_ASSERT( old_handle.get() != this, "Old handle should not be the current buffer anymore!" );
		new_handle = this->shared_from_this();

		for ( auto& [ old_allocation, new_allocation ] : allocations )
		{
			old_allocation->m_parent_buffer = old_handle;
			new_allocation->m_parent_buffer = new_handle;
		}

		//Now we need to transfer the data from the old buffer to the new buffer
		for ( const auto& allocation : allocations )
		{
			const auto& [ old_suballocation, new_suballocation ] = allocation;
			TransferManager::getInstance().copySuballocationRegion( old_suballocation, new_suballocation );
		}
	}
	*/

	std::shared_ptr< BufferSuballocationHandle > BufferHandle::
		allocate( vk::DeviceSize desired_memory_size, const vk::DeviceSize t_alignment )
	{
		ZoneScoped;
		//Calculate alignment from alignment, ubo_alignment, and atom_size_alignment
		desired_memory_size = align( desired_memory_size, alignment() );

		if ( !canAllocate( desired_memory_size, t_alignment ) )
		{
			return { nullptr };
		}

		auto itter { findAvailableBlock( desired_memory_size, t_alignment ) };

		if ( itter == m_free_blocks.end() )
		{
			// Could not find a block that is available.
			return { nullptr };
		}

		auto [ selected_block_offset, selected_block_size ] = *itter;
		m_free_blocks.erase( itter );

		assert( selected_block_offset <= this->size() );
		assert( selected_block_size <= this->size() );

		const auto aligned_offset { align( selected_block_offset, alignment(), t_alignment ) };
		FGL_ASSERT( aligned_offset % combineAlignment( alignment(), t_alignment ) == 0, "Alignment failed!" );

		//Fix the offset and size if they aren't alligned
		if ( aligned_offset != selected_block_offset )
		{
			//Insert the space left over before the block starts back into the free blocks
			const std::size_t leftover_start_size { aligned_offset - selected_block_offset };

			m_free_blocks.emplace_back( selected_block_offset, leftover_start_size );

			mergeFreeBlocks();

			selected_block_offset = aligned_offset;
			assert( selected_block_size >= leftover_start_size );
			selected_block_size -= leftover_start_size;
		}

		// Add the suballocation
		// m_allocations.insert_or_assign( selected_block_offset, desired_memory_size );
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

		std::ranges::
			remove_if( m_active_suballocations, []( auto& suballocation ) -> bool { return suballocation.expired(); } );

		auto suballocation_handle { std::make_shared< BufferSuballocationHandle >(
			Buffer( this->shared_from_this() ), selected_block_offset, desired_memory_size, t_alignment ) };

		m_active_suballocations.push_back( suballocation_handle );

		return suballocation_handle;
	}

	bool BufferHandle::canAllocate( const vk::DeviceSize memory_size, const vk::DeviceSize alignment )
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

		return true;
	}

	void BufferHandle::mergeFreeBlocks()
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

	void BufferHandle::setDebugName( const std::string& str )
	{
		m_debug_name = str;
		std::string sized_name { std::format( "{}: {}", m_debug_name, literals::size_literals::toString( size() ) ) };

		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eBuffer;
		info.pObjectName = sized_name.c_str();
		info.objectHandle = reinterpret_cast< std::uint64_t >( static_cast< VkBuffer >( this->m_buffer ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

	void BufferHandle::free( BufferSuballocationHandle& info )
	{
		ZoneScoped;

		if ( info.offset() >= this->size() ) throw std::runtime_error( "Offset was outside of bounds of buffer" );
		if ( info.offset() + info.size() > this->size() )
			throw std::runtime_error(
				std::format(
					"offset() + size() was outside the bounds of the buffer ({} + {} == {} >= {})",
					info.offset(),
					info.size(),
					info.offset() + info.size(),
					size() ) );

		//Add the block back to the free blocks
		m_free_blocks.emplace_back( info.offset(), info.size() );

		mergeFreeBlocks();

#ifndef NDEBUG
		//Check that we haven't lost any memory
		std::size_t sum { 0 };
		for ( const auto& size : this->m_free_blocks | std::views::values )
		{
			sum += size;
		}

		for ( auto& suballocation : m_active_suballocations )
		{
			if ( suballocation.expired() ) continue;
			sum += suballocation.lock()->size();
		}

		if ( sum != this->size() )
			throw std::runtime_error(
				std::format( "Memory leaked! Expected {} was {}: Lost {}", this->size(), sum, this->size() - sum ) );
#endif
	}

	vk::DeviceSize BufferHandle::used() const
	{
		vk::DeviceSize total_size { 0 };

		for ( auto& suballocation : m_active_suballocations )
		{
			if ( suballocation.expired() ) continue;
			total_size += suballocation.lock()->size();
		}

		return total_size;
	}

	vk::DeviceSize BufferHandle::largestBlock() const
	{
		vk::DeviceSize largest { 0 };

		for ( const auto& size : m_free_blocks | std::views::values )
		{
			largest = std::max( largest, size );
		}

		return largest;
	}

} // namespace fgl::engine::memory
