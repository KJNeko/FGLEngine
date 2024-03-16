//
// Created by kj16609 on 3/11/24.
//

#include "CompositionSystem.hpp"

#include <engine/texture/Texture.hpp>

namespace fgl::engine
{

	CompositionSystem::CompositionSystem( [[maybe_unused]] Device& device, VkRenderPass render_pass )
	{
		PipelineConfigInfo composition_info { render_pass };
		PipelineConfigInfo::addColorAttachmentConfig( composition_info );
		PipelineConfigInfo::disableVertexInput( composition_info );
		composition_info.subpass = 1;

		m_pipeline = std::make_unique< CompositionPipeline >( Device::getInstance(), composition_info );
		m_pipeline->setDebugName( "Composition pipeline" );
	}

	vk::CommandBuffer& CompositionSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };
		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, GBufferDescriptorSet::m_set_idx, info.gbuffer_descriptor_set );

		return info.command_buffer;
	}

	void CompositionSystem::pass( FrameInfo& info )
	{
		info.command_buffer.nextSubpass( vk::SubpassContents::eInline );
		auto& command_buffer { setupSystem( info ) };

		command_buffer.draw( 3, 1, 0, 0 );
	}

} // namespace fgl::engine