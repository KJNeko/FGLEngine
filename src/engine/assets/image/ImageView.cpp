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

	ImageView::ImageView( const std::shared_ptr< ImageHandle >& img ) :
	  m_resource( img ),
	  m_descriptor_info(),
	  m_image_view( createImageView( img ) ),
	  m_sampler(),
	  m_name( "Unnamed ImageView" )
	{
		m_descriptor_info.imageLayout = img->m_final_layout;
		m_descriptor_info.imageView = m_image_view;
	}

	ImageView::ImageView( const std::shared_ptr< ImageHandle >& img, Sampler&& sampler ) : ImageView( img )
	{
		m_sampler = std::move( sampler );
	}

	vk::DescriptorImageInfo ImageView::descriptorInfo( const vk::Sampler sampler, const vk::ImageLayout layout ) const
	{
		vk::DescriptorImageInfo info { descriptorInfo( layout ) };

		info.sampler = sampler;

		return info;
	}

	vk::DescriptorImageInfo ImageView::descriptorInfo( const vk::ImageLayout layout ) const
	{
		vk::DescriptorImageInfo info {};
		info.imageLayout = layout;
		info.imageView = m_image_view;

		return info;
	}

	ImageView::~ImageView()
	{}

	void ImageView::setName( const std::string& str )
	{
		m_name = str;
		m_sampler.setName( str + " Sampler" );
		m_resource->setName( str + " Resource" );

		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eImageView;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< std::uint64_t >( static_cast< VkImageView >( *m_image_view ) ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

	vk::ImageView ImageView::getVkView() const
	{
		return *m_image_view;
	}

	vk::Extent2D ImageView::getExtent() const
	{
		return m_resource->extent();
	}

	bool ImageView::ready() const
	{
		return m_resource->ready();
	}

} // namespace fgl::engine
