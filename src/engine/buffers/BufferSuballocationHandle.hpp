//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace vk::raii
{
	class CommandBuffer;
}

namespace fgl::engine
{
	class Buffer;

	struct BufferSuballocationHandle
	{
		Buffer& buffer;

		//! Size of the buffer this suballocation is a part of
		vk::DeviceSize m_size;

		//! Offset within buffer
		vk::DeviceSize m_offset;

		void* mapped { nullptr };

		bool m_staged { false };

		BufferSuballocationHandle() = delete;
		BufferSuballocationHandle( const BufferSuballocationHandle& ) = delete;
		BufferSuballocationHandle& operator=( const BufferSuballocationHandle& ) = delete;

		vk::Buffer getBuffer();

		BufferSuballocationHandle( BufferSuballocationHandle&& ) = delete;
		BufferSuballocationHandle& operator=( BufferSuballocationHandle&& ) = delete;

		BufferSuballocationHandle( Buffer& buffer, vk::DeviceSize memory_size, vk::DeviceSize offset );
		~BufferSuballocationHandle();

		vk::BufferCopy copyRegion( BufferSuballocationHandle& target );

		void copyTo( vk::raii::CommandBuffer& cmd_buffer, BufferSuballocationHandle& other );

		vk::Buffer getVkBuffer() const;

		bool ready() const { return m_staged; }

		void setReady( const bool value ) { m_staged = value; }

		vk::DeviceSize getOffset() const { return m_offset; }
	};

} // namespace fgl::engine
