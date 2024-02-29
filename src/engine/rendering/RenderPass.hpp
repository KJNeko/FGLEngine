//
// Created by kj16609 on 12/30/23.
//

#pragma once

#include <unordered_map>

#include "Subpass.hpp"
#include "engine/image/ImageView.hpp"

namespace fgl::engine
{

	struct RenderPassResources
	{
		std::vector< std::vector< std::shared_ptr< ImageView > > > m_image_views;

		RenderPassResources() = delete;
		RenderPassResources( const RenderPassResources& ) = delete;

		RenderPassResources( std::vector< std::vector< std::shared_ptr< ImageView > > >&& image_views ) :
		  m_image_views( std::move( image_views ) )
		{}

		std::vector< vk::ImageView > forFrame( std::uint32_t frame ) const
		{
			std::vector< vk::ImageView > views;

			for ( auto& view : m_image_views.at( frame ) ) views.push_back( view->getVkView() );

			return views;
		}

		~RenderPassResources() { std::cout << "Cleaing resources" << std::endl; }
	};

	class RenderPass
	{
		std::vector< vk::AttachmentDescription > attachment_descriptions {};
		std::vector< vk::ClearValue > m_clear_values {};

		std::vector< vk::SubpassDescription > subpass_descriptions {};
		std::vector< vk::SubpassDependency > dependencies {};

		std::vector< AttachmentResources > m_attachment_resources {};

	  public:

		std::vector< vk::ClearValue > getClearValues() const { return m_clear_values; }

		template < typename SubpassT >
			requires is_subpass< SubpassT >
		void registerSubpass( SubpassT& subpass )
		{
			subpass_descriptions.emplace_back( subpass.description() );

			for ( auto& dependency : subpass.dependencies )
			{
				dependencies.push_back( dependency );
			}
		}

		template < is_attachment... Attachments >
		void registerAttachments( Attachments&... attachments )
		{
			attachment_descriptions.reserve( sizeof...( Attachments ) );
			m_clear_values.reserve( sizeof...( Attachments ) );

			( ( attachments.setIndex( static_cast< std::uint32_t >( attachment_descriptions.size() ) ),
			    attachment_descriptions.push_back( attachments.desc() ),
			    m_clear_values.push_back( attachments.m_clear_value ) ),
			  ... );

			m_attachment_resources.reserve( sizeof...( Attachments ) );

			( ( m_attachment_resources.emplace_back( attachments.m_attachment_resources ) ), ... );
		}

		std::unique_ptr< RenderPassResources > resources( std::uint16_t frame_count )
		{
			assert( m_attachment_resources.size() > 0 && "Must register attachments before getting resources" );

			//Each attachment will have a vector of image views, one for each frame
			// We need to seperate them out so that we have a vector of image views for each frame
			std::vector< std::vector< std::shared_ptr< ImageView > > > views;

			views.resize( frame_count );

			for ( auto& attachment : m_attachment_resources )
			{
				assert(
					attachment.m_image_views.size() == frame_count
					&& "Attachment image views must be equal to frame count" );
				for ( std::uint16_t frame_idx = 0; frame_idx < attachment.m_image_views.size(); ++frame_idx )
				{
					views[ frame_idx ].emplace_back( attachment.m_image_views[ frame_idx ] );
				}
			}

			return std::make_unique< RenderPassResources >( std::move( views ) );
		}

		vk::RenderPass create();
	};

} // namespace fgl::engine