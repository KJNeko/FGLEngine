//
// Created by kj16609 on 6/23/24.
//

#include "GuiSystem.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/assets/model/SimpleVertex.hpp"
#include "engine/rendering/pipelines/v2/AttachmentBuilder.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/pipelines/v2/PipelineBuilder.hpp"

namespace fgl::engine
{

	GuiSystem::GuiSystem( vk::raii::RenderPass& render_pass )
	{
		//descriptors::DescriptorSetCollection descriptors { gui_descriptor_set };

		PipelineBuilder builder { render_pass, 0 };

		builder.addDescriptorSet( gui_descriptor_set );

		builder.setAttributeDescriptions( SimpleVertex::getAttributeDescriptions() );
		builder.setBindingDescriptions( SimpleVertex::getBindingDescriptions() );

		builder.setVertexShader( Shader::loadVertex( "shaders/fullscreen.vert" ) );
		builder.setFragmentShader( Shader::loadFragment( "shaders/gui-compose.frag" ) );

		builder.addColorAttachment().finish();

		m_pipeline = builder.create();
		m_pipeline->setDebugName( "Gui Pipeline" );
	}

	vk::raii::CommandBuffer& GuiSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };

		m_pipeline->bind( command_buffer );

		// m_pipeline->bindDescriptor( command_buffer, 0, info.gui_input_descriptor );

		return command_buffer;
	}

	void GuiSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "GuiSystem::pass" );
		auto& command_buffer { setupSystem( info ) };

		command_buffer.draw( 3, 1, 0, 0 );
	}

} // namespace fgl::engine
