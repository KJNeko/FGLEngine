//
// Created by kj16609 on 1/2/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

#include "Image.hpp"
#include "ImageHandle.hpp"
#include "Sampler.hpp"
#include "engine/concepts/is_image.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{

	class ImageView
	{
		std::shared_ptr< ImageHandle > m_resource;

		Sampler m_sampler;

		vk::DescriptorImageInfo m_descriptor_info {};

		vk::ImageView m_image_view { VK_NULL_HANDLE };

	  public:

		void setName( const std::string str );

		ImageView() = delete;
		ImageView( const ImageView& ) = delete;
		ImageView& operator=( const ImageView& ) = delete;

		ImageView( ImageView&& other ) noexcept :
		  m_resource( std::move( other.m_resource ) ),
		  m_descriptor_info( std::move( other.m_descriptor_info ) ),
		  m_image_view( std::move( other.m_image_view ) ),
		  m_sampler(
			  vk::Filter::eLinear,
			  vk::Filter::eLinear,
			  vk::SamplerMipmapMode::eLinear,
			  vk::SamplerAddressMode::eClampToEdge )
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

		vk::Extent2D getExtent() const;

		vk::ImageView& getVkView();

		vk::Image& getVkImage();

		Sampler& getSampler() { return m_sampler; };

		vk::DescriptorImageInfo descriptorInfo( vk::Sampler sampler, vk::ImageLayout layout ) const;
		vk::DescriptorImageInfo descriptorInfo( vk::ImageLayout layout ) const;
	};

} // namespace fgl::engine
