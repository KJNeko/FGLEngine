//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

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

	  public:

		BufferSuballocationHandle() = delete;
		BufferSuballocationHandle( const BufferSuballocationHandle& ) = delete;
		BufferSuballocationHandle& operator=( const BufferSuballocationHandle& ) = delete;
		BufferSuballocationHandle( BufferSuballocationHandle&& ) = delete;
		BufferSuballocationHandle& operator=( BufferSuballocationHandle&& ) = delete;

		BufferSuballocationHandle( Buffer& buffer, vk::DeviceSize memory_size, vk::DeviceSize offset );
		~BufferSuballocationHandle();

		vk::Buffer getVkBuffer() const;

		vk::DeviceSize getOffset() const { return m_offset; }
	};

} // namespace fgl::engine
