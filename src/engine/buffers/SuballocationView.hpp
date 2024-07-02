//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

namespace fgl::engine::memory
{

	struct BufferSuballocationHandle;

	class SuballocationView
	{
		std::shared_ptr< BufferSuballocationHandle > m_suballocation;
		vk::DeviceSize m_offset;
		vk::DeviceSize m_size;

	  public:

		void setOffset( vk::DeviceSize offset );

		SuballocationView(
			std::shared_ptr< BufferSuballocationHandle > handle, vk::DeviceSize offset, vk::DeviceSize size ) :
		  m_suballocation( handle ),
		  m_offset( offset ),
		  m_size( size )
		{}

		//! Returns the buffer
		vk::Buffer getVkBuffer();

		//! Returns the offset of this view within the buffer
		vk::DeviceSize getOffset();
	};

} // namespace fgl::engine