//
// Created by kj16609 on 1/2/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include "ImageHandle.hpp"
#include "Sampler.hpp"

namespace fgl::engine
{

	class ImageView
	{
		std::shared_ptr< ImageHandle > m_resource;

		vk::DescriptorImageInfo m_descriptor_info;

		vk::raii::ImageView m_image_view;

		Sampler m_sampler;

	  public:

		void setName( const std::string& str );

		//! Returns true if the resource has been staged
		bool ready();

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

		vk::raii::ImageView createImageView( const std::shared_ptr< ImageHandle >& img );
		ImageView( std::shared_ptr< ImageHandle >& img );

		vk::Extent2D getExtent() const;

		vk::raii::ImageView& getVkView();

		VkImageView operator*() { return *m_image_view; }

		VkImage getVkImage() { return m_resource->getVkImage(); }

		Sampler& getSampler() { return m_sampler; };

		vk::DescriptorImageInfo descriptorInfo( vk::Sampler sampler, vk::ImageLayout layout ) const;
		vk::DescriptorImageInfo descriptorInfo( vk::ImageLayout layout ) const;
	};

} // namespace fgl::engine
