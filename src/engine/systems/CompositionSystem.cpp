//
// Created by kj16609 on 3/11/24.
//

#include "CompositionSystem.hpp"

#include <engine/texture/Texture.hpp>

namespace fgl::engine
{

	CompositionSystem::CompositionSystem( [[maybe_unused]] Device& device, vk::raii::RenderPass& render_pass )
	{
		PipelineConfigInfo composition_info { render_pass };
		PipelineConfigInfo::addColorAttachmentConfig( composition_info );
		PipelineConfigInfo::disableVertexInput( composition_info );
		PipelineConfigInfo::disableCulling( composition_info );
		composition_info.subpass = 1;

		m_composite_pipeline =
			std::make_unique< CompositionPipeline >( Device::getInstance(), std::move( composition_info ) );
		m_composite_pipeline->setDebugName( "Composition pipeline" );
	}

	vk::raii::CommandBuffer& CompositionSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };

		command_buffer.nextSubpass( vk::SubpassContents::eInline );

		m_composite_pipeline->bind( command_buffer );

		m_composite_pipeline
			->bindDescriptor( command_buffer, GBufferDescriptorSet::m_set_idx, info.gbuffer_descriptor_set );

		return info.command_buffer;
	}

	void CompositionSystem::pass( FrameInfo& info )
	{
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Composition Pass" );

		command_buffer.draw( 3, 1, 0, 0 );
	}

} // namespace fgl::engine