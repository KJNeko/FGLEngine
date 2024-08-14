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

		ImageView( std::shared_ptr< ImageHandle >& img );

		ImageView( const ImageView& ) = delete;
		ImageView& operator=( const ImageView& ) = delete;

		ImageView( ImageView&& other ) = default;
		ImageView& operator=( ImageView&& other ) = default;

		[[nodiscard]] static vk::raii::ImageView createImageView( const std::shared_ptr< ImageHandle >& img );

		[[nodiscard]] vk::Extent2D getExtent() const;

		[[nodiscard]] vk::ImageView getVkView();

		[[nodiscard]] VkImageView operator*() { return *m_image_view; }

		[[nodiscard]] VkImage getVkImage() { return m_resource->getVkImage(); }

		[[nodiscard]] Sampler& getSampler() { return m_sampler; };

		vk::DescriptorImageInfo descriptorInfo( vk::Sampler sampler, vk::ImageLayout layout ) const;
		vk::DescriptorImageInfo descriptorInfo( vk::ImageLayout layout ) const;
	};

} // namespace fgl::engine
