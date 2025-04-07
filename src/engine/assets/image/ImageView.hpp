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

		vk::raii::ImageView m_image_view;

		Sampler m_sampler;
		std::string m_name;

		[[nodiscard]] static vk::raii::ImageView createImageView( const std::shared_ptr< ImageHandle >& img );

		[[nodiscard]] vk::DescriptorImageInfo descriptorInfo( vk::Sampler sampler, vk::ImageLayout layout ) const;

	  public:

		//! Returns true if the resource has been staged
		bool ready() const;

		ImageView() = delete;

		explicit ImageView( const std::shared_ptr< ImageHandle >& img );
		explicit ImageView( const std::shared_ptr< ImageHandle >& img, Sampler&& sampler );

		ImageView( const ImageView& ) = delete;
		ImageView& operator=( const ImageView& ) = delete;

		ImageView( ImageView&& other ) = default;
		ImageView& operator=( ImageView&& other ) = default;

		[[nodiscard]] vk::Extent2D getExtent() const;

		[[nodiscard]] vk::ImageView getVkView() const;

		[[nodiscard]] VkImageView operator*() const { return *m_image_view; }

		[[nodiscard]] VkImage getVkImage() const { return m_resource->getVkImage(); }

		// void setSampler( Sampler&& sampler ) { m_sampler = std::forward< Sampler >( sampler ); }
		[[nodiscard]] const Sampler& getSampler() const { return m_sampler; };

		[[nodiscard]] vk::DescriptorImageInfo descriptorInfo( vk::ImageLayout layout ) const;

		~ImageView();

		void setName( const std::string& str );
	};

} // namespace fgl::engine
