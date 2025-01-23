//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

#include "engine/debug/Track.hpp"

namespace vk::raii
{
	class CommandBuffer;
}

namespace fgl::engine::memory
{
	class Buffer;

	struct BufferSuballocationHandle
	{
		Buffer& m_buffer;

		debug::Track< "GPU", "BufferSuballocationHandle" > m_track {};

		//! Size of the buffer this suballocation is a part of
		vk::DeviceSize m_size;

		//! Offset within buffer
		vk::DeviceSize m_offset;

		void* m_ptr { nullptr };

		bool m_staged { false };

		BufferSuballocationHandle() = delete;

		BufferSuballocationHandle( Buffer& p_buffer, vk::DeviceSize offset, vk::DeviceSize memory_size );
		~BufferSuballocationHandle();

		BufferSuballocationHandle( const BufferSuballocationHandle& ) = delete;
		BufferSuballocationHandle& operator=( const BufferSuballocationHandle& ) = delete;

		BufferSuballocationHandle( BufferSuballocationHandle&& ) = delete;
		BufferSuballocationHandle& operator=( BufferSuballocationHandle&& ) = delete;

		[[nodiscard]] vk::Buffer getBuffer() const;
		[[nodiscard]] vk::Buffer getVkBuffer() const;

		[[nodiscard]] vk::BufferCopy copyRegion( const BufferSuballocationHandle& target, std::size_t offset ) const;

		vk::DeviceSize getOffset() const { return m_offset; }

		void copyTo(
			const vk::raii::CommandBuffer& cmd_buffer,
			const BufferSuballocationHandle& other,
			std::size_t offset ) const;

		bool ready() const { return m_staged; }

		void setReady( const bool value ) { m_staged = value; }
	};

} // namespace fgl::engine::memory
