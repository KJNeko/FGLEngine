//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

namespace fgl::engine
{
	namespace memory
	{
		class TransferManager;
	}

	class ImageView;
	class ImageHandle;

	class Image
	{
		std::shared_ptr< ImageHandle > m_handle;
		std::weak_ptr< ImageView > m_view {};
		vk::Extent2D m_extent;

		friend class memory::TransferManager;

	  public:

		Image() = delete;

		[[nodiscard]]
		Image( vk::Extent2D extent, vk::Format format, vk::Image image, vk::ImageUsageFlags usage ) noexcept;

		[[nodiscard]] Image(
			vk::Extent2D extent,
			vk::Format format,
			vk::ImageUsageFlags usage,
			vk::ImageLayout inital_layout,
			vk::ImageLayout final_layout );

		Image( const Image& other ) : m_handle( other.m_handle ), m_extent( other.m_extent ) {}

		[[nodiscard]] Image& operator=( const Image& other );

		VkImage getVkImage() const;

		[[nodiscard]] Image( Image&& other ) = default;

		[[nodiscard]] Image& operator=( Image&& other ) noexcept;

		Image& setName( const std::string& str );

		const vk::Extent2D& getExtent() const { return m_extent; }

		[[nodiscard]] std::shared_ptr< ImageView > getView();

		vk::ImageMemoryBarrier transitionTo(
			vk::ImageLayout old_layout, vk::ImageLayout new_layout, const vk::ImageSubresourceRange& range ) const;

		vk::ImageMemoryBarrier
			transitionTo( vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageAspectFlags aspect );

		inline vk::ImageMemoryBarrier transitionColorTo( vk::ImageLayout old_layout, vk::ImageLayout new_layout )
		{
			return transitionTo( old_layout, new_layout, vk::ImageAspectFlagBits::eColor );
		}
	};

} // namespace fgl::engine