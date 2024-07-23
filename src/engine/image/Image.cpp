//
// Created by kj16609 on 12/4/23.
//

#include "Image.hpp"

#include "ImageView.hpp"

namespace fgl::engine
{

	std::shared_ptr< ImageView > Image::getView()
	{
		if ( !view.expired() )
			return view.lock();
		else
		{
			assert( m_handle );
			auto ptr { std::make_shared< ImageView >( m_handle ) };
			view = ptr;
			return ptr;
		}
	}

	Image::Image(
		const vk::Extent2D extent,
		const vk::Format format,
		const vk::Image image,
		const vk::ImageUsageFlags usage ) noexcept :
	  m_handle( std::make_shared< ImageHandle >( extent, format, image, usage ) ),
	  m_extent( extent )
	{}

	Image::Image(
		const vk::Extent2D extent,
		const vk::Format format,
		const vk::ImageUsageFlags usage,
		const vk::ImageLayout inital_layout,
		const vk::ImageLayout final_layout ) :
	  m_handle( std::make_shared< ImageHandle >( extent, format, usage, inital_layout, final_layout ) ),
	  m_extent( extent )
	{}

	Image& Image::operator=( const Image& other )
	{
		m_handle = other.m_handle;
		view = {};
		return *this;
	}

	VkImage Image::getVkImage() const
	{
		return m_handle->getVkImage();
	}

	Image& Image::operator=( Image&& other ) noexcept
	{
		m_handle = std::move( other.m_handle );
		view = std::move( other.view );
		m_extent = other.m_extent;
		return *this;
	}

	Image& Image::setName( const std::string& str )
	{
		m_handle->setName( str );
		return *this;
	}

} // namespace fgl::engine