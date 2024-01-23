//
// Created by kj16609 on 1/4/24.
//

#include "ImageHandle.hpp"

namespace fgl::engine
{

	ImageHandle::ImageHandle( fgl::engine::ImageHandle&& other ) noexcept :
	  m_allocation( other.m_allocation ),
	  m_allocation_info( other.m_allocation_info ),
	  m_image( other.m_image ),
	  m_extent( other.m_extent ),
	  m_format( other.m_format ),
	  m_usage( other.m_usage )
	{
		other.m_allocation = VK_NULL_HANDLE;
		other.m_image = VK_NULL_HANDLE;
		other.m_allocation_info = {};
		other.m_image = VK_NULL_HANDLE;
	}

	ImageHandle::ImageHandle(
		const vk::Extent2D extent, const vk::Format format, vk::Image image, vk::ImageUsageFlags usage ) noexcept :
	  m_image( image ),
	  m_extent( extent ),
	  m_format( format ),
	  m_usage( usage )
	{}

	ImageHandle::ImageHandle(
		const vk::Extent2D extent,
		const vk::Format format,
		vk::ImageUsageFlags usage,
		vk::ImageLayout inital_layout,
		vk::ImageLayout final_layout ) :
	  m_extent( extent ),
	  m_format( format ),
	  m_usage( usage ),
	  m_initial_layout( inital_layout ),
	  m_final_layout( final_layout )
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

		if ( Device::getInstance().device().createImage( &image_info, nullptr, &m_image ) != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to create image" );

		VmaAllocationCreateInfo alloc_info {};
		alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		if ( vmaAllocateMemoryForImage(
				 Device::getInstance().allocator(), getVkImage(), &alloc_info, &m_allocation, &m_allocation_info )
		     != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate memory for image" );

		if ( vmaBindImageMemory( Device::getInstance().allocator(), m_allocation, getVkImage() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to bind memory" );
	}

	void ImageHandle::setName( const std::string str )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};

		info.objectType = vk::ObjectType::eImage;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< uint64_t >( static_cast< VkImage >( m_image ) ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}
} // namespace fgl::engine
