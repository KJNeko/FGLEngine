//
// Created by kj16609 on 1/4/24.
//

#pragma once

#include <memory>

#include "engine/logging/logging.hpp"
#include "engine/rendering/Device.hpp"
#include "vma/vma_impl.hpp"
#include "vulkan/vulkan.hpp"

namespace fgl::engine
{
	class ImageView;

	class ImageHandle
	{
		VmaAllocation m_allocation { VK_NULL_HANDLE };
		VmaAllocationInfo m_allocation_info {};

		vk::Extent2D m_extent;
		vk::Format m_format;
		vk::ImageUsageFlags m_usage;

		vk::ImageLayout m_initial_layout { vk::ImageLayout::eUndefined };
		vk::ImageLayout m_final_layout { vk::ImageLayout::eUndefined };

		// Because of the way the swapchain works we need to be able to storage a `VkImage` handle.
		std::variant< vk::raii::Image, vk::Image > m_image;

		friend class ImageView;
		friend class Image;

	  public:

		FGL_DELETE_ALL_Ro5( ImageHandle );

		ImageHandle(
			const vk::Extent2D extent, const vk::Format format, vk::Image image, vk::ImageUsageFlags usage ) noexcept;

		ImageHandle(
			const vk::Extent2D extent,
			const vk::Format format,
			vk::ImageUsageFlags usage,
			vk::ImageLayout inital_layout,
			vk::ImageLayout final_layout );

		void setName( const std::string str );

		VkImage operator*()
		{
			ZoneScoped;
			if ( std::holds_alternative< vk::raii::Image >( m_image ) )
			{
				return *std::get< vk::raii::Image >( m_image );
			}
			else
				return std::get< vk::Image >( m_image );
		}

		VkImage getVkImage() { return **this; }

		vk::Format format() const { return m_format; }

		vk::Extent2D extent() const { return m_extent; }

		vk::ImageAspectFlags aspectMask() const
		{
			vk::ImageAspectFlags flags {};

			if ( m_usage & vk::ImageUsageFlagBits::eColorAttachment ) flags |= vk::ImageAspectFlagBits::eColor;

			//TODO: This is likely wrong since not all sampled images will be a color.
			// Should look into the various image flags and figure out a better pattern here
			if ( m_usage & vk::ImageUsageFlagBits::eSampled ) flags |= vk::ImageAspectFlagBits::eColor;

			if ( m_usage & vk::ImageUsageFlagBits::eDepthStencilAttachment )
				flags |= ( vk::ImageAspectFlagBits::eDepth );

			return flags;
		}

		~ImageHandle()
		{
			if ( m_allocation != VK_NULL_HANDLE )
			{
				vmaFreeMemory( Device::getInstance().allocator(), m_allocation );
			}
		}
	};

} // namespace fgl::engine