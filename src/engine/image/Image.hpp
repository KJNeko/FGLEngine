//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "engine/image/ImageHandle.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{
	class ImageView;

	class Image
	{
		std::shared_ptr< ImageHandle > m_handle {};
		std::weak_ptr< ImageView > view {};

	  public:

		Image() = delete;

		Image( const vk::Extent2D extent, const vk::Format format, vk::Image image, vk::ImageUsageFlags usage ) noexcept
		  :
		  m_handle( std::make_shared< ImageHandle >( extent, format, image, usage ) )
		{}

		[[nodiscard]] vk::Image& getVkImage();

		Image& setName( const std::string str );

		Image(
			const vk::Extent2D extent,
			const vk::Format format,
			vk::ImageUsageFlags usage,
			vk::ImageLayout inital_layout,
			vk::ImageLayout final_layout ) :
		  m_handle( std::make_shared< ImageHandle >( extent, format, usage, inital_layout, final_layout ) )
		{}

		Image( Image&& other ) = default;

		Image( const Image& other ) : m_handle( other.m_handle ), view() {}

		Image& operator=( const Image& other )
		{
			m_handle = other.m_handle;
			view = {};
			return *this;
		}

		Image& operator=( Image&& other ) noexcept
		{
			m_handle = std::move( other.m_handle );
			view = std::move( other.view );
			return *this;
		}

		[[nodiscard]] std::shared_ptr< ImageView > getView();
	};

} // namespace fgl::engine