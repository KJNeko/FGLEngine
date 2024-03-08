//
// Created by kj16609 on 11/30/23.
//

#pragma once

#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "align.hpp"
#include "engine/literals/size.hpp"
#include "vma/vma_impl.hpp"

namespace fgl::engine::exceptions
{
	struct EngineError : public std::runtime_error
	{
		EngineError( const char* msg ) : std::runtime_error( msg ) {}
	};

	struct AllocationException : public EngineError
	{
		AllocationException() : EngineError( "Failed to allocate memory" ) {}
	};

} // namespace fgl::engine::exceptions

namespace fgl::engine
{

	class BufferHandle;
	class BufferSuballocation;
	struct BufferSuballocationHandle;

	//TODO: Dynamic/onDemand resizing of Buffer for suballocations
	//TODO: Defragmentation

	class Device;

	class BufferHandle
	{
		vk::Buffer m_buffer { VK_NULL_HANDLE };
		VmaAllocation m_allocation {};
		VmaAllocationInfo m_alloc_info {};

		vk::DeviceSize m_memory_size;

		vk::BufferUsageFlags m_usage;
		vk::MemoryPropertyFlags m_memory_properties;

		void alloc( vk::DeviceSize memory_size );
		void dealloc();

	  public:

		BufferHandle() = delete;
		BufferHandle( const BufferHandle& other ) = delete;
		BufferHandle& operator=( const BufferHandle& other ) = delete;
		BufferHandle( BufferHandle&& other ) = delete;
		BufferHandle& operator=( BufferHandle&& other ) = delete;

		friend class Buffer;

		BufferHandle(
			vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		~BufferHandle();

		auto address() const { return m_alloc_info.deviceMemory; }

		auto size() const { return m_alloc_info.size; }
	};

	class Buffer
	{
		inline static std::vector< std::weak_ptr< BufferHandle > > m_buffer_handles {};

		std::shared_ptr< BufferHandle > m_handle;

	  public:

		vk::BufferUsageFlags m_usage;

		vk::MemoryPropertyFlags m_memory_properties;

		Buffer() = delete;
		Buffer( vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		~Buffer() = default;

		Buffer( const Buffer& other ) = delete;
		Buffer& operator=( const Buffer& other ) = delete;

		Buffer( Buffer&& other ) = default;
		Buffer& operator=( Buffer&& other ) = default;

		static std::vector< std::weak_ptr< BufferHandle > > getActiveBufferHandles();

		inline vk::Buffer& getVkBuffer() noexcept { return m_handle->m_buffer; }

		//! Returns the required alignment for this buffer.
		vk::DeviceSize alignment();

	  private:

		//! @brief List of all active suballocations
		//! <offset, size>
		std::map< vk::DeviceSize, vk::DeviceSize > m_suballocations {};

		//! @brief list of any free blocks
		//! @note All blocks are amalgamated to the largest they can expand to.
		//! <offset, size>
		std::vector< std::pair< vk::DeviceSize, vk::DeviceSize > > m_free_blocks {};

	  public:

		//! Returns the vulkan buffer handle for this buffer
		vk::Buffer getBuffer() const
		{
			assert( m_handle );
			assert( m_handle->m_buffer != VK_NULL_HANDLE );

			return m_handle->m_buffer;
		}

		//! Returns the vulkan memory handle for this buffer
		vk::DeviceMemory getMemory() const
		{
			assert( m_handle );
			assert( m_handle->m_alloc_info.deviceMemory != VK_NULL_HANDLE );

			return m_handle->m_alloc_info.deviceMemory;
		}

		//! Total memory size of the buffer
		vk::DeviceSize size() const
		{
			assert( m_handle );
			assert( !std::isnan( m_handle->m_memory_size ) );

			return m_handle->m_memory_size;
		}

		void* map( BufferSuballocationHandle& info );

		bool isMappable() const
		{
			assert( m_handle );

			return m_handle->m_alloc_info.pMappedData != nullptr;
		}

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
			suballocate( vk::DeviceSize memory_size, std::uint32_t alignment = 1 );

		void free( BufferSuballocationHandle& info );

		void mergeFreeBlocks();
	};

	void initGlobalStagingBuffer( std::uint64_t size );
	Buffer& getGlobalStagingBuffer();
} // namespace fgl::engine
