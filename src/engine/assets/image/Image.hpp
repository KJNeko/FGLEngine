//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <memory>

#include "FGL_DEFINES.hpp"
#include "Sampler.hpp"
#include "debug/Track.hpp"

namespace fgl::engine
{
	namespace memory
	{
		class TransferManager;
	}

	class ImageView;
	class ImageHandle;

	class Image : public std::enable_shared_from_this< Image >
	{
		std::shared_ptr< ImageHandle > m_handle;
		std::weak_ptr< ImageView > m_view {};
		vk::Extent2D m_extent;

		debug::Track< "GPU", "Image" > m_track {};

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

		[[nodiscard]] const vk::Extent2D& getExtent() const { return m_extent; }

		[[nodiscard]] std::shared_ptr< ImageView > getView( Sampler sampler = {} );

		[[nodiscard]] vk::ImageMemoryBarrier transitionTo(
			vk::ImageLayout old_layout, vk::ImageLayout new_layout, const vk::ImageSubresourceRange& range ) const;

		[[nodiscard]] vk::ImageMemoryBarrier
			transitionTo( vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::ImageAspectFlags aspect );

		FGL_FORCE_INLINE vk::ImageMemoryBarrier
			transitionColorTo( const vk::ImageLayout old_layout, const vk::ImageLayout new_layout )
		{
			return transitionTo( old_layout, new_layout, vk::ImageAspectFlagBits::eColor );
		}
	};

} // namespace fgl::engine