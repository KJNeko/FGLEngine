//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <vector>

#include "engine/concepts/is_attachment.hpp"
#include "engine/image/Image.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{

	struct AttachmentResources
	{
		std::vector< std::shared_ptr< Image > > m_images {};
		std::vector< std::shared_ptr< ImageView > > m_image_views {};
	};

	template <
		vk::AttachmentLoadOp load_op,
		vk::AttachmentStoreOp store_op,
		vk::ImageLayout inital_layout,
		vk::ImageLayout final_layout,
		vk::ImageUsageFlags usage >
	class Attachment
	{
		vk::AttachmentDescription description {};
		std::uint32_t index { std::numeric_limits< std::uint32_t >::max() };

	  public:

		vk::ClearValue m_clear_value {};

		void setClear( vk::ClearColorValue value ) { m_clear_value = value; }

		void setClear( vk::ClearDepthStencilValue value ) { m_clear_value = value; }

		AttachmentResources m_attachment_resources {};

		void setIndex( const std::uint32_t idx ) { index = idx; }

		constexpr static vk::AttachmentLoadOp loadOp = load_op;
		constexpr static vk::AttachmentStoreOp storeOp = store_op;
		constexpr static vk::ImageLayout InitalLayout = inital_layout;
		constexpr static vk::ImageLayout FinalLayout = final_layout;

		Attachment( const vk::Format format )
		{
			assert( format != vk::Format::eUndefined && "Attachment format must not be undefined" );
			description.format = format;
			description.samples = vk::SampleCountFlagBits::e1;
			description.loadOp = load_op;
			description.storeOp = store_op;
			description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			description.initialLayout = inital_layout;
			description.finalLayout = final_layout;
		}

		void attachImageView( std::uint16_t frame_idx, std::shared_ptr< ImageView > image_view )
		{
			auto& image_views = m_attachment_resources.m_image_views;
			if ( image_views.size() <= frame_idx ) image_views.resize( frame_idx + 1 );
			image_views[ frame_idx ] = std::move( image_view );
		}

		void linkImage( std::uint16_t frame_idx, Image& image ) { attachImageView( frame_idx, image.getView() ); }

		void linkImages( std::vector< Image >& images )
		{
			for ( std::uint16_t i = 0; i < images.size(); ++i )
			{
				linkImage( i, images[ i ] );
			}
		}

		void createResources( const std::uint32_t count, vk::Extent2D extent )
		{
			for ( std::uint16_t i = 0; i < count; ++i )
			{
				auto& images { m_attachment_resources.m_images };
				auto& image_views { m_attachment_resources.m_image_views };
				auto& itter { images.emplace_back( std::make_shared< Image >(
					extent,
					description.format,
					usage | vk::ImageUsageFlagBits::eInputAttachment,
					inital_layout,
					final_layout ) ) };
				image_views.emplace_back( itter->getView() );
			}
		}

		//! Creates a resource that is used across all frames
		void createResourceSpread( const std::uint32_t count, vk::Extent2D extent )
		{
			auto image { std::make_shared< Image >(
				extent,
				description.format,
				usage | vk::ImageUsageFlagBits::eInputAttachment,
				inital_layout,
				final_layout ) };
			for ( std::uint32_t i = 0; i < count; ++i )
			{
				m_attachment_resources.m_images.emplace_back( image );
				m_attachment_resources.m_image_views.emplace_back( image->getView() );
			}
		}

		AttachmentResources resources() { return m_attachment_resources; }

		vk::AttachmentDescription& desc() { return description; }

		std::uint32_t getIndex() const
		{
			assert(
				index != std::numeric_limits< std::uint32_t >::max()
				&& "Attachment must be registered in RenderPass before use" );
			return index;
		}

		friend class RenderPass;
	};

	template < is_attachment AttachmentT, vk::ImageLayout layout >
	struct InputAttachment
	{
		static constexpr bool is_input { true };
		static constexpr vk::ImageLayout m_layout { layout };

		using Attachment = AttachmentT;
	};

	template < is_attachment AttachmentT, vk::ImageLayout layout >
	struct UsedAttachment
	{
		static constexpr bool is_input { false };
		static constexpr vk::ImageLayout m_layout { layout };

		using Attachment = AttachmentT;
	};

	template < typename T >
	concept is_input_attachment = requires( T a ) {
		{
			a.is_input
		} -> std::same_as< const bool& >;
		{
			a.m_layout
		} -> std::same_as< const vk::ImageLayout& >;
	} && T::is_input;

	template < typename T >
	concept is_used_attachment = requires( T a ) {
		{
			a.is_input
		} -> std::same_as< const bool& >;
		{
			a.m_layout
		} -> std::same_as< const vk::ImageLayout& >;
	} && !T::is_input;

	template < typename T > concept is_wrapped_attachment = is_input_attachment< T > || is_used_attachment< T >;

	template < typename T >
		requires is_wrapped_attachment< T >
	using UnwrappedAttachment = std::conditional_t< is_wrapped_attachment< T >, typename T::Attachment, T >;

	using ColoredPresentAttachment = Attachment<
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR,
		vk::ImageUsageFlagBits::eColorAttachment >;

	using DepthAttachment = Attachment<
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment >;

	using ColorAttachment = Attachment<
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageUsageFlagBits::eColorAttachment >;

	static_assert( is_input_attachment< InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > > );

} // namespace fgl::engine
