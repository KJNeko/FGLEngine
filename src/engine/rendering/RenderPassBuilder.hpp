//
// Created by kj16609 on 12/30/23.
//

#pragma once

#include "Subpass.hpp"
#include "engine/image/ImageView.hpp"

namespace fgl::engine
{

	class RenderPassBuilder
	{
		std::vector< vk::AttachmentDescription > attachment_descriptions {};
		std::vector< vk::ClearValue > m_clear_values {};

		std::vector< vk::SubpassDescription > subpass_descriptions {};
		std::vector< vk::SubpassDependency > dependencies {};

	  public:

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
			static_assert(
				sizeof...( Attachments ) == maxIndex< Attachments... >() + 1,
				"There must be no empty attachment indicies when creating a render pass" );

			attachment_descriptions.resize( sizeof...( Attachments ) );
			m_clear_values.resize( sizeof...( Attachments ) );

			( ( attachment_descriptions[ attachments.m_index ] = attachments.desc() ), ... );
			( ( m_clear_values[ attachments.m_index ] = attachments.m_clear_value ), ... );
		}

		vk::raii::RenderPass create();
	};

} // namespace fgl::engine