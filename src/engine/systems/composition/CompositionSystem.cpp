//
// Created by kj16609 on 3/11/24.
//

#include "CompositionSystem.hpp"

#include "Control.hpp"
#include "editor/src/gui/safe_include.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/rendering/RenderingFormats.hpp"
#include "engine/rendering/pipelines/v2/AttachmentBuilder.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/pipelines/v2/PipelineBuilder.hpp"

namespace fgl::engine
{

	CompositionSystem::CompositionSystem( vk::raii::RenderPass& render_pass )
	{
		PipelineBuilder builder { 0 };

		builder.addDescriptorSet( gbuffer_set );
		builder.addDescriptorSet( Camera::getDescriptorLayout() );

		builder.addColorAttachment().setFormat( pickColorFormat() ).finish();

		builder.setPushConstant( vk::ShaderStageFlagBits::eFragment, sizeof( CompositionControl ) );

		builder.setVertexShader( Shader::loadVertex( "shaders/fullscreen.vert" ) );
		builder.setFragmentShader( Shader::loadFragment( "shaders/composition.frag" ) );

		builder.disableCulling();
		builder.disableVertexInput();

		m_composite_pipeline = builder.create();

		m_composite_pipeline->setDebugName( "Composition pipeline" );
	}

	CompositionSystem::~CompositionSystem()
	{}

	vk::raii::CommandBuffer& CompositionSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };

		m_composite_pipeline->bind( command_buffer );

		m_composite_pipeline->bindDescriptor( command_buffer, info.getGBufferDescriptor() );
		m_composite_pipeline->bindDescriptor( command_buffer, info.getCameraDescriptor() );

		ImGui::Begin( "Composition" );

		ImGui::InputInt( "Selection", &m_control.m_flags );

		ImGui::End();

		m_composite_pipeline->pushConstant( command_buffer, vk::ShaderStageFlagBits::eFragment, m_control );

		return info.command_buffer;
	}

	void CompositionSystem::pass( FrameInfo& info )
	{
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Composition Pass" );

		command_buffer.draw( 3, 1, 0, 0 );
	}

} // namespace fgl::engine