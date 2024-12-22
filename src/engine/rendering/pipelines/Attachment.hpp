//
// Created by kj16609 on 12/4/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <vector>

#include "engine/assets/image/Image.hpp"
#include "engine/concepts/is_attachment.hpp"
#include "engine/rendering/types.hpp"

namespace fgl::engine
{

	struct AttachmentResources
	{
		std::vector< std::shared_ptr< Image > > m_images {};
		std::vector< std::shared_ptr< ImageView > > m_image_views {};
	};

	template <
		std::uint32_t Index,
		vk::AttachmentLoadOp load_op,
		vk::AttachmentStoreOp store_op,
		vk::ImageLayout inital_layout,
		vk::ImageLayout final_layout,
		vk::ImageUsageFlags usage >
	class Attachment
	{
		vk::AttachmentDescription description {};

	  public:

		vk::ClearValue m_clear_value {};

		void setClear( vk::ClearColorValue value ) { m_clear_value = value; }

		void setClear( vk::ClearDepthStencilValue value ) { m_clear_value = value; }

		void setName( const char* str )
		{
			for ( const auto& image : m_attachment_resources.m_images )
			{
				image->setName( str );
			}
		}

		AttachmentResources m_attachment_resources {};

		constexpr static vk::AttachmentLoadOp loadOp { load_op };
		constexpr static vk::AttachmentStoreOp storeOp { store_op };
		constexpr static vk::ImageLayout InitalLayout { inital_layout };
		constexpr static vk::ImageLayout FinalLayout { final_layout };
		constexpr static std::uint32_t m_index { Index };

		constexpr Attachment( const vk::Format format )
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
			assert( images.size() > 0 );
			for ( std::uint16_t i = 0; i < images.size(); ++i )
			{
				linkImage( i, images[ i ] );
			}
		}

		void createResources(
			const std::uint32_t count, vk::Extent2D extent, vk::ImageUsageFlags extra_flags = vk::ImageUsageFlags( 0 ) )
		{
			for ( std::uint16_t i = 0; i < count; ++i )
			{
				auto& images { m_attachment_resources.m_images };
				auto& image_views { m_attachment_resources.m_image_views };
				const auto& itter { images.emplace_back(
					std::make_shared< Image >(
						extent,
						description.format,
						usage | vk::ImageUsageFlagBits::eInputAttachment | extra_flags,
						vk::ImageLayout::eUndefined,
						final_layout ) ) };
				image_views.emplace_back( itter->getView() );
			}
		}

		//! Creates a resource that is used across all frames
		void createResourceSpread(
			const std::uint32_t count, vk::Extent2D extent, vk::ImageUsageFlags extra_flags = vk::ImageUsageFlags( 0 ) )
		{
			auto image { std::make_shared< Image >(
				extent,
				description.format,
				usage | vk::ImageUsageFlagBits::eInputAttachment | extra_flags,
				vk::ImageLayout::eUndefined,
				final_layout ) };
			for ( std::uint32_t i = 0; i < count; ++i )
			{
				m_attachment_resources.m_images.emplace_back( image );
				m_attachment_resources.m_image_views.emplace_back( image->getView() );
			}
		}

		ImageView& getView( const FrameIndex frame_idx ) const
		{
			assert( frame_idx < m_attachment_resources.m_image_views.size() );
			return *m_attachment_resources.m_image_views[ frame_idx ];
		}

		Image& getImage( const FrameIndex frame_idx ) const
		{
			assert( frame_idx < m_attachment_resources.m_image_views.size() );
			return *m_attachment_resources.m_images[ frame_idx ];
		}

		vk::RenderingAttachmentInfo renderInfo( const FrameIndex frame_index, const vk::ImageLayout layout ) const
		{
			vk::RenderingAttachmentInfo info {};

			info.setClearValue( m_clear_value );
			info.imageView = getView( frame_index ).getVkView();
			info.loadOp = load_op;
			info.storeOp = store_op;
			info.imageLayout = layout;

			return info;
		}

		constexpr vk::AttachmentDescription& desc() { return description; }

		friend class RenderPassBuilder;
	};

	template < std::size_t index, vk::ImageLayout layout >
	struct InputAttachment
	{
		static constexpr bool is_input { true };
		static constexpr vk::ImageLayout m_layout { layout };
		static constexpr std::size_t m_index { index };
	};

	template < std::size_t index, vk::ImageLayout layout >
	struct UsedAttachment
	{
		static constexpr vk::ImageLayout m_layout { layout };
		static constexpr std::size_t m_index { index };
		static constexpr bool is_input { false };
	};

	template < typename T >
	concept is_input_attachment = requires( T a ) {
		{ a.is_input } -> std::same_as< const bool& >;
		{ a.m_layout } -> std::same_as< const vk::ImageLayout& >;
	} && T::is_input;

	template < typename T >
	concept is_used_attachment = requires( T a ) {
		{ a.is_input } -> std::same_as< const bool& >;
		{ a.m_layout } -> std::same_as< const vk::ImageLayout& >;
	} && !T::is_input;

	template < typename T > concept is_wrapped_attachment = is_input_attachment< T > || is_used_attachment< T >;

	template < typename T >
		requires is_wrapped_attachment< T >
	using UnwrappedAttachment = std::conditional_t< is_wrapped_attachment< T >, typename T::Attachment, T >;

	//! Checks if the wrapped attachment is a depth attachment
	template < typename T >
	concept is_wrapped_depth_attachment = is_wrapped_attachment< T > && is_attachment< UnwrappedAttachment< T > >
	                                   && ( T::m_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal );

	//Helper functions
	template < is_attachment Attachment, is_attachment... Attachments >
	consteval std::uint32_t maxIndex()
	{
		if constexpr ( sizeof...( Attachments ) == 0 )
		{
			return Attachment::m_index;
		}
		else
		{
			return std::max( Attachment::m_index, maxIndex< Attachments... >() );
		}
	}

	template < is_attachment... Attachments >
	static std::vector< vk::ImageView > getViewsForFrame( const FrameIndex frame_idx, Attachments... attachments )
	{
		std::vector< vk::ImageView > view {};
		view.resize( sizeof...( Attachments ) );
		static_assert( maxIndex< Attachments... >() + 1 == sizeof...( Attachments ) );

		( ( view[ attachments.m_index ] = *attachments.getView( frame_idx ) ), ... );

		return view;
	}

	template < is_attachment... Attachments >
	static std::vector< vk::ClearValue > gatherClearValues( Attachments... attachments )
	{
		std::vector< vk::ClearValue > clear_values {};
		clear_values.resize( sizeof...( Attachments ) );
		static_assert( maxIndex< Attachments... >() + 1 == sizeof...( Attachments ) );

		( ( clear_values[ attachments.m_index ] = attachments.m_clear_value ), ... );

		return clear_values;
	}

	template < std::uint32_t Index >
	using ColoredPresentAttachment = Attachment<
		Index,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR,
		vk::ImageUsageFlagBits::eColorAttachment >;

	template < std::uint32_t Index >
	using DepthAttachment = Attachment<
		Index,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment >;

#if ENABLE_IMGUI
	constexpr vk::ImageUsageFlags IMGUI_ATTACHMENT_FLAGS { vk::ImageUsageFlagBits::eSampled };
#else
	constexpr vk::ImageUsageFlags IMGUI_ATTACHMENT_FLAGS { 0 };
#endif

	template < std::uint32_t Index >
	using ColorAttachment = Attachment<
		Index,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | IMGUI_ATTACHMENT_FLAGS >;

	template < std::uint32_t Index >
	using InputColorAttachment = Attachment<
		Index,
		vk::AttachmentLoadOp::eLoad,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | IMGUI_ATTACHMENT_FLAGS >;

} // namespace fgl::engine
