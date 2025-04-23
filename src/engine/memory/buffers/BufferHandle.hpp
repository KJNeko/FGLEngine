//
// Created by kj16609 on 11/30/23.
//

#pragma once

// clang-format off
#include <vulkan/vulkan.hpp>
#include <tracy/Tracy.hpp>
// clang-format on

#include <cassert>
#include <cmath>
#include <memory>
#include <stacktrace>
#include <unordered_map>
#include <utility>

#include "engine/debug/Track.hpp"
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
	class BufferHandle : public std::enable_shared_from_this< BufferHandle >
	{
		vk::Buffer m_buffer { VK_NULL_HANDLE };
		VmaAllocation m_allocation {};
		VmaAllocationInfo m_alloc_info {};

		debug::Track< "GPU", "Buffer" > m_track {};

		vk::DeviceSize m_memory_size;

		vk::BufferUsageFlags m_usage;
		vk::MemoryPropertyFlags m_memory_properties;

		std::vector< std::weak_ptr< BufferSuballocationHandle > > m_active_suballocations {};
		std::unordered_map< vk::DeviceSize, std::stacktrace > m_allocation_traces {};
		int m_resize_counter { 0 };

		//! @brief List of all active suballocations
		//! <offset, size>
		using AllocationSize = vk::DeviceSize;

		//! @brief list of any free blocks
		//! @note All blocks are amalgamated to the largest they can expand to.
		//! <offset, size>
		std::vector< std::pair< vk::DeviceSize, vk::DeviceSize > > m_free_blocks {};

	  public:

		std::weak_ptr< BufferHandle > m_old_handle {};
		std::string m_debug_name { "Debug name" };
		std::string m_pool_name { std::format( "GPU {} Suballocation", m_debug_name ) };

	  private:

		static std::tuple< vk::Buffer, VmaAllocationInfo, VmaAllocation > allocBuffer(
			vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property_flags );
		void deallocBuffer( const vk::Buffer&, const VmaAllocation& );

		BufferHandle() = delete;
		BufferHandle( const BufferHandle& other ) = delete;
		BufferHandle& operator=( const BufferHandle& other ) = delete;
		BufferHandle( BufferHandle&& other ) = delete;
		BufferHandle& operator=( BufferHandle&& other ) = delete;

		void swap( BufferHandle& other ) noexcept;

	  public:

		BufferHandle(
			vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		~BufferHandle();

		auto address() const { return m_alloc_info.deviceMemory; }

		auto size() const { return m_alloc_info.size; }

		vk::DeviceSize largestBlock() const;

		vk::DeviceSize used() const;

		bool isHostVisible() const;

		bool needsFlush() const;

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

		bool isMappable() const { return m_alloc_info.pMappedData != nullptr; }

		void rebindDescriptors();
		void resize( vk::DeviceSize new_size );
		//! Returns a allocation block from this buffer. Block will be aligned with nonUniformBufferOffsetAlignment
		//! and nonCoherentAtomSize if required (is_uniform_buffer and is_host_visible respectively)

		/**
		 * @param desired_memory_size Size of each N
		 * @param alignment The alignment to use.
		 * @param source_loc Source location.
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
			allocate( vk::DeviceSize desired_memory_size, std::uint32_t alignment = 1 );

		bool canAllocate( vk::DeviceSize memory_size, std::uint32_t alignment = 1 );

		//! Frees a given suballocation. After calling this the handle is invalid and accessing it is UB
		void free( BufferSuballocationHandle& info );

		void mergeFreeBlocks();

		void setDebugName( const std::string& str );

	  private:

		void* map( const BufferSuballocationHandle& handle ) const;

		//! Returns the required alignment for this buffer.
		vk::DeviceSize alignment() const;

		decltype( m_free_blocks )::iterator findAvailableBlock( vk::DeviceSize memory_size, std::uint32_t t_alignment );
	};

	struct Buffer final : public std::shared_ptr< BufferHandle >
	{
		Buffer( vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		Buffer( const std::shared_ptr< BufferHandle >& buffer );

		~Buffer();
	};

	std::vector< std::shared_ptr< BufferHandle > > getActiveBuffers();
} // namespace fgl::engine::memory