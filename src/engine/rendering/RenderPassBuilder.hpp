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
		}

		vk::raii::RenderPass create();
	};

} // namespace fgl::engine