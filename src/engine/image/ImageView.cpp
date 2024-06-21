//
// Created by kj16609 on 1/5/24.
//

#include "ImageView.hpp"

namespace fgl::engine
{

	vk::raii::ImageView ImageView::createImageView( const std::shared_ptr< ImageHandle >& img )
	{
		vk::ImageViewCreateInfo info {};
		info.image = **img;
		info.viewType = vk::ImageViewType::e2D;
		info.format = img->format();

		info.subresourceRange.aspectMask = img->aspectMask();

		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		return Device::getInstance()->createImageView( info );
	}

	ImageView::ImageView( std::shared_ptr< ImageHandle >& img ) :
	  m_resource( img ),
	  m_descriptor_info(),
	  m_sampler(),
	  m_image_view( createImageView( img ) )
	{
		m_descriptor_info.imageLayout = img->m_final_layout;
		m_descriptor_info.imageView = m_image_view;
	}

	vk::DescriptorImageInfo ImageView::descriptorInfo( vk::Sampler sampler, vk::ImageLayout layout ) const
	{
		vk::DescriptorImageInfo info { descriptorInfo( layout ) };

		info.sampler = sampler;

		return info;
	}

	vk::DescriptorImageInfo ImageView::descriptorInfo( vk::ImageLayout layout ) const
	{
		vk::DescriptorImageInfo info {};
		info.imageLayout = layout;
		info.imageView = m_image_view;

		return info;
	}

	vk::raii::ImageView& ImageView::getVkView()
	{
		return m_image_view;
	}

	vk::Extent2D ImageView::getExtent() const
	{
		return m_resource->extent();
	}

	void ImageView::setName( const std::string str )
	{
		m_resource->setName( str );
	}

} // namespace fgl::engine
