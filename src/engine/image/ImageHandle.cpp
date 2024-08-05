//
// Created by kj16609 on 1/4/24.
//

#include "ImageHandle.hpp"

namespace fgl::engine
{

	//! Creates an image handle from a pre-existing vk::Image object
	ImageHandle::ImageHandle(
		const vk::Extent2D extent, const vk::Format format, vk::Image image, const vk::ImageUsageFlags usage ) noexcept
	  :
	  m_extent( extent ),
	  m_format( format ),
	  m_usage( usage ),
	  m_image( image ),
	  m_staged( true ) // Set staged to be true since we don't need to stage this image.
	{
		assert( std::get< vk::Image >( m_image ) != VK_NULL_HANDLE );
	}

	//! Creates an image using the Device
	vk::raii::Image createImage(
		const vk::Extent2D extent,
		const vk::Format format,
		const vk::ImageLayout inital_layout,
		const vk::ImageUsageFlags usage )
	{
		vk::ImageCreateInfo image_info {};

		image_info.imageType = vk::ImageType::e2D;

		image_info.extent.width = extent.width;
		image_info.extent.height = extent.height;
		image_info.extent.depth = 1;

		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = format;
		image_info.tiling = vk::ImageTiling::eOptimal;
		image_info.initialLayout = inital_layout;

		image_info.usage = usage;

		image_info.samples = vk::SampleCountFlagBits::e1;
		image_info.sharingMode = vk::SharingMode::eExclusive;

		return Device::getInstance()->createImage( image_info );
	}

	//! Constructs a new image allocated from memory.
	ImageHandle::ImageHandle(
		const vk::Extent2D extent,
		const vk::Format format,
		const vk::ImageUsageFlags usage,
		const vk::ImageLayout inital_layout,
		const vk::ImageLayout final_layout ) :
	  m_extent( extent ),
	  m_format( format ),
	  m_usage( usage ),
	  m_initial_layout( inital_layout ),
	  m_final_layout( final_layout ),
	  m_image( createImage( extent, format, inital_layout, usage ) ),
	  m_staged( true )
	{
		assert( std::holds_alternative< vk::raii::Image >( m_image ) );
		assert( *std::get< vk::raii::Image >( m_image ) != VK_NULL_HANDLE );

		ZoneScoped;
		//Allocate memory for image
		VmaAllocationCreateInfo alloc_info {};
		alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if ( const auto result = vmaAllocateMemoryForImage(
				 Device::getInstance().allocator(), getVkImage(), &alloc_info, &m_allocation, &m_allocation_info );
		     result != VK_SUCCESS )
		{
			log::critical(
				"Failed to allocate memory for image of size ({},{}) with error {}",
				m_extent.width,
				m_extent.height,
				static_cast< int >( result ) );
			throw std::runtime_error( "Failed to allocate memory for image" );
		}

		if ( vmaBindImageMemory( Device::getInstance().allocator(), m_allocation, getVkImage() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to bind memory" );
	}

	void ImageHandle::setName( const std::string str )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};

		info.objectType = vk::ObjectType::eImage;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< uint64_t >( getVkImage() ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}
} // namespace fgl::engine
