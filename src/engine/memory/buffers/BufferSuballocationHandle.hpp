//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "BufferHandle.hpp"
#include "engine/debug/Track.hpp"

namespace vk::raii
{
	class CommandBuffer;
}

namespace fgl::engine::memory
{
	class BufferHandle;

	struct BufferSuballocationHandle : public std::enable_shared_from_this< BufferSuballocationHandle >
	{
		Buffer m_parent_buffer;

		debug::Track< "GPU", "BufferSuballocationHandle" > m_track {};

		//! Size of the buffer this suballocation is a part of
		vk::DeviceSize m_size;

		//! Offset within buffer
		vk::DeviceSize m_offset;

		//! Alignment used when allocating
		vk::DeviceSize m_alignment;

		void* m_ptr { nullptr };

		bool m_staged { false };

		BufferSuballocationHandle(
			const Buffer& p_buffer, vk::DeviceSize offset, vk::DeviceSize memory_size, vk::DeviceSize alignment );

		FGL_DELETE_DEFAULT_CTOR( BufferSuballocationHandle );
		FGL_DELETE_MOVE( BufferSuballocationHandle );
		FGL_DELETE_COPY( BufferSuballocationHandle );

		~BufferSuballocationHandle();

		[[nodiscard]] vk::Buffer getBuffer() const;
		[[nodiscard]] vk::Buffer getVkBuffer() const;

		[[nodiscard]] vk::BufferCopy copyRegion( const BufferSuballocationHandle& target, std::size_t offset ) const;

		[[nodiscard]] vk::DeviceSize getOffset() const { return m_offset; }

		void copyTo(
			const vk::raii::CommandBuffer& cmd_buffer,
			const BufferSuballocationHandle& other,
			std::size_t offset ) const;

		bool ready() const { return m_staged; }

		void setReady( const bool value ) { m_staged = value; }
	};

} // namespace fgl::engine::memory
