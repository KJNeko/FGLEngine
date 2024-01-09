//
// Created by kj16609 on 1/2/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "Image.hpp"
#include "ImageHandle.hpp"
#include "engine/Device.hpp"
#include "engine/concepts/is_image.hpp"

namespace fgl::engine
{

	struct ImageView
	{
		std::shared_ptr< ImageHandle > m_resource;

		vk::DescriptorImageInfo m_descriptor_info {};

		vk::ImageView m_image_view { VK_NULL_HANDLE };

		ImageView() = delete;
		ImageView( const ImageView& ) = delete;
		ImageView& operator=( const ImageView& ) = delete;

		ImageView( ImageView&& other ) noexcept :
		  m_resource( std::move( other.m_resource ) ),
		  m_descriptor_info( std::move( other.m_descriptor_info ) ),
		  m_image_view( std::move( other.m_image_view ) )
		{
			other.m_image_view = VK_NULL_HANDLE;
		}

		~ImageView()
		{
			if ( m_image_view )
			{
				Device::getInstance().device().destroyImageView( m_image_view );
			}
		}

		ImageView( std::shared_ptr< ImageHandle >& img );

		vk::ImageView& view() { return m_image_view; }

		vk::DescriptorImageInfo descriptorInfo( vk::Sampler sampler, vk::ImageLayout layout ) const;
	};

} // namespace fgl::engine
