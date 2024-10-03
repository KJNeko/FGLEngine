//
// Created by kj16609 on 11/30/23.
//

#pragma once

#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.hpp>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>

#include "vma/vma_impl.hpp"

namespace fgl::engine
{
	class Device;

	namespace gui
	{
		struct AllocationList;
		AllocationList getTotalAllocated();
	} // namespace gui
} // namespace fgl::engine

namespace fgl::engine::memory
{
	class BufferSuballocation;
	struct BufferSuballocationHandle;

	//TODO: Dynamic/onDemand resizing of Buffer for suballocations
	//TODO: Defragmentation

	//TODO: Ensure this class can't be directly accessed from within Buffer unless we are trying
	// to access it in a debug manner (IE the drawStats menu)
	class Buffer
	{
		vk::Buffer m_buffer { VK_NULL_HANDLE };
		VmaAllocation m_allocation {};
		VmaAllocationInfo m_alloc_info {};

		vk::DeviceSize m_memory_size;

		vk::BufferUsageFlags m_usage;
		vk::MemoryPropertyFlags m_memory_properties;

	  public:

		std::string m_debug_name { "Debug name" };

	  private:

		void alloc( vk::DeviceSize memory_size );
		void dealloc();

		Buffer() = delete;
		Buffer( const Buffer& other ) = delete;
		Buffer& operator=( const Buffer& other ) = delete;
		Buffer( Buffer&& other ) = delete;
		Buffer& operator=( Buffer&& other ) = delete;

	  public:

		Buffer( vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		~Buffer();

		auto address() const { return m_alloc_info.deviceMemory; }

		auto size() const { return m_alloc_info.size; }

		vk::DeviceSize largestBlock() const;

		vk::DeviceSize used() const;

	  private:

		void* map( BufferSuballocationHandle& handle );

		//! Returns the required alignment for this buffer.
		vk::DeviceSize alignment();

		//! @brief List of all active suballocations
		//! <offset, size>
		std::map< vk::DeviceSize, vk::DeviceSize > m_allocations {};

		//! @brief list of any free blocks
		//! @note All blocks are amalgamated to the largest they can expand to.
		//! <offset, size>
		std::vector< std::pair< vk::DeviceSize, vk::DeviceSize > > m_free_blocks {};

		decltype( m_free_blocks )::iterator findAvailableBlock( vk::DeviceSize memory_size, std::uint32_t t_alignment );

	  public:

		//! Returns the vulkan buffer handle for this buffer
		vk::Buffer getVkBuffer() const { return m_buffer; }

		//! Returns the vulkan memory handle for this buffer
		vk::DeviceMemory getMemory() const
		{
			assert( m_alloc_info.deviceMemory != VK_NULL_HANDLE );

			return m_alloc_info.deviceMemory;
		}

		friend struct BufferSuballocationHandle;
		friend class BufferSuballocation; //TODO: Remove this

		friend gui::AllocationList gui::getTotalAllocated();

	  public:

		bool isMappable() const { return m_alloc_info.pMappedData != nullptr; }

		//! Returns a allocation block from this buffer. Block will be aligned with nonUniformBufferOffsetAlignment
		//! and nonCoherentAtomSize if required (is_uniform_buffer and is_host_visible respectively)
		/**
		 * @param memory_size Size of each N
		 * @param alignment The alignment to use.
		 * @return
		 *
		 * @note Alignment is forced to be at least the size of the largest alignment required by the device.
		 * (`alignment` vs `nonCoherentAtomSize` vs `minUniformBufferOffsetAlignment`)
		 * @par
		 * @note Alignment for ubo is 0 if the buffer is not a uniform buffer
		 * @par
		 * @note Alignment for atom_size is 0 if buffer is not host visible
		 */
		std::shared_ptr< BufferSuballocationHandle >
			allocate( vk::DeviceSize memory_size, std::uint32_t alignment = 1 );

		bool canAllocate( vk::DeviceSize memory_size, std::uint32_t alignment = 1 );

		//! Frees a given suballocation. After calling this the handle is invalid and accessing it is UB
		void free( BufferSuballocationHandle& info );

		void mergeFreeBlocks();

		void setDebugName( const std::string str );
	};

	std::vector< Buffer* > getActiveBuffers();
} // namespace fgl::engine::memory