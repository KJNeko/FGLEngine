//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "engine/Device.hpp"
#include "engine/image/ImageHandle.hpp"

namespace fgl::engine
{
	struct ImageView;

	class Image
	{
		std::shared_ptr< ImageHandle > m_handle;

	  public:

		Image() = delete;

		Image( const Image& ) = delete;
		Image& operator=( const Image& ) = delete;

		Image( Image&& ) noexcept = default;
		Image& operator=( Image&& ) noexcept = default;

		Image( const vk::Extent2D extent, const vk::Format format, vk::Image image, vk::ImageUsageFlags usage ) noexcept
		  :
		  m_handle( std::make_shared< ImageHandle >( extent, format, image, usage ) )
		{}

		void setName( const std::string str );

		Image(
			const vk::Extent2D extent,
			const vk::Format format,
			vk::ImageUsageFlags usage,
			vk::ImageLayout inital_layout,
			vk::ImageLayout final_layout ) :
		  m_handle( std::make_shared< ImageHandle >( extent, format, usage, inital_layout, final_layout ) )
		{}

		std::shared_ptr< ImageView > view();
	};

} // namespace fgl::engine