//
// Created by kj16609 on 1/5/24.
//

#include "ImageView.hpp"

namespace fgl::engine
{

	ImageView::ImageView( std::shared_ptr< ImageHandle >& img ) :
	  m_resource( img ),
	  m_descriptor_info(),
	  m_image_view( VK_NULL_HANDLE ),
	  m_sampler()
	{
		vk::ImageViewCreateInfo view_info {};
		view_info.image = img->getVkImage();
		view_info.viewType = vk::ImageViewType::e2D;
		view_info.format = img->format();

		view_info.subresourceRange.aspectMask = img->aspectMask();

		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		m_descriptor_info.imageLayout = img->m_final_layout;
		m_descriptor_info.imageView = m_image_view;

		if ( Device::getInstance().device().createImageView( &view_info, nullptr, &m_image_view )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to create image view" );
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

	vk::ImageView& ImageView::getVkView()
	{
		return m_image_view;
	}

	vk::Image& ImageView::getVkImage()
	{
		return m_resource->getVkImage();
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
