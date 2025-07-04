//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <queue>

#include "BufferHandle.hpp"
#include "engine/debug/Track.hpp"

namespace vk::raii
{
	class CommandBuffer;
}

namespace fgl::engine::memory
{
	class BufferHandle;

	class BufferSuballocationHandle : public std::enable_shared_from_this< BufferSuballocationHandle >
	{
		friend class BufferSuballocation;

		Buffer m_parent_buffer;

		debug::Track< "GPU", "BufferSuballocationHandle" > m_track {};

		//! Size of the buffer this suballocation is a part of
		vk::DeviceSize m_size;

		//! Offset within buffer
		vk::DeviceSize m_offset;

		//! Alignment used when allocating
		vk::DeviceSize m_alignment;

		void* m_ptr { nullptr };

		bool m_reallocated { false };
		std::shared_ptr< BufferSuballocationHandle > m_reallocated_to { nullptr };

		bool m_staged { false };
		std::vector< std::weak_ptr< BufferSuballocationHandle > > m_dependents {};

	  public:

		BufferSuballocationHandle(
			const Buffer& p_buffer, vk::DeviceSize offset, vk::DeviceSize memory_size, vk::DeviceSize alignment );

		FGL_DELETE_DEFAULT_CTOR( BufferSuballocationHandle );
		FGL_DELETE_MOVE( BufferSuballocationHandle );
		FGL_DELETE_COPY( BufferSuballocationHandle );

		~BufferSuballocationHandle();

		void copyTo(
			const vk::raii::CommandBuffer& cmd_buffer,
			const BufferSuballocationHandle& other,
			std::size_t offset ) const;

		void flagReallocated( const std::shared_ptr< BufferSuballocationHandle >& shared )
		{
			m_reallocated = true;
			m_reallocated_to = shared;
		}

		std::pair< std::shared_ptr< BufferSuballocationHandle >, std::shared_ptr< BufferSuballocationHandle > >
			reallocate( const std::shared_ptr< BufferHandle >& shared );

		bool reallocated() const { return m_reallocated; }

		std::shared_ptr< BufferSuballocationHandle > reallocatedTo() const { return m_reallocated_to; }

		void markSource( const std::shared_ptr< BufferSuballocationHandle >& source );

		void setReady( bool value );

		[[nodiscard]] vk::BufferCopy
			copyRegion( const BufferSuballocationHandle& target, std::size_t suballocation_offset ) const;

		[[nodiscard]] vk::Buffer getBuffer() const;
		[[nodiscard]] vk::Buffer getVkBuffer() const;

		vk::DeviceSize offset() const { return m_offset; }

		//! True if there are no pending writes
		bool stable() const;
		//! True if the data in the buffer is valid (Written to at least once)
		bool ready() const;

		vk::DeviceSize size() const { return m_size; }

		vk::DeviceSize alignment() const { return m_alignment; }
	};

} // namespace fgl::engine::memory
