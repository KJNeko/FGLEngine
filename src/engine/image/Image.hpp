//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <memory>

#include "engine/image/ImageHandle.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{
	namespace memory
	{
		class TransferManager;
	}

	class ImageView;

	class Image
	{
		std::shared_ptr< ImageHandle > m_handle;
		std::weak_ptr< ImageView > view {};

		friend class memory::TransferManager;

	  public:

		Image() = delete;

		Image( const vk::Extent2D extent, const vk::Format format, vk::Image image, vk::ImageUsageFlags usage ) noexcept
		  :
		  m_handle( std::make_shared< ImageHandle >( extent, format, image, usage ) )
		{}

		Image& setName( const std::string str );

		Image(
			const vk::Extent2D extent,
			const vk::Format format,
			vk::ImageUsageFlags usage,
			vk::ImageLayout inital_layout,
			vk::ImageLayout final_layout ) :
		  m_handle( std::make_shared< ImageHandle >( extent, format, usage, inital_layout, final_layout ) )
		{}

		Image( const Image& other ) : m_handle( other.m_handle ) {}

		Image& operator=( const Image& other )
		{
			m_handle = other.m_handle;
			view = {};
			return *this;
		}

		Image( Image&& other ) = default;

		Image& operator=( Image&& other ) noexcept
		{
			m_handle = std::move( other.m_handle );
			view = std::move( other.view );
			return *this;
		}

		[[nodiscard]] std::shared_ptr< ImageView > getView();
	};

} // namespace fgl::engine