//
// Created by kj16609 on 3/11/24.
//

#include "CompositionSystem.hpp"

#include "engine/rendering/pipelines/v2/AttachmentBuilder.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/pipelines/v2/PipelineBuilder.hpp"

namespace fgl::engine
{

	CompositionSystem::CompositionSystem( vk::raii::RenderPass& render_pass )
	{
		constexpr std::size_t SUBPASS { 1 };

		PipelineBuilder builder { render_pass, SUBPASS };

		builder.addDescriptorSet( gbuffer_set );

		builder.addColorAttachment().finish();

		builder.setVertexShader( Shader::loadVertex( "shaders/fullscreen.vert" ) );
		builder.setFragmentShader( Shader::loadFragment( "shaders/composition.frag" ) );

		builder.disableCulling();
		builder.disableVertexInput();

		m_composite_pipeline = builder.create();

		m_composite_pipeline->setDebugName( "Composition pipeline" );
	}

	vk::raii::CommandBuffer& CompositionSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };

		command_buffer.nextSubpass( vk::SubpassContents::eInline );

		m_composite_pipeline->bind( command_buffer );

		m_composite_pipeline->bindDescriptor( command_buffer, info.getGBufferDescriptor() );

		return info.command_buffer;
	}

	void CompositionSystem::pass( FrameInfo& info )
	{
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Composition Pass" );

		command_buffer.draw( 3, 1, 0, 0 );
	}

} // namespace fgl::engine