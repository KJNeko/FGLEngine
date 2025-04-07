//
// Created by kj16609 on 6/23/24.
//

#include "GuiSystem.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/rendering/pipelines/v2/AttachmentBuilder.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/pipelines/v2/PipelineBuilder.hpp"
#include "rendering/RenderingFormats.hpp"

namespace fgl::engine
{

	GuiSystem::GuiSystem()
	{
		//descriptors::DescriptorSetCollection descriptors { gui_descriptor_set };

		PipelineBuilder builder { 0 };

		builder.addDescriptorSet( gui_descriptor_set );

		// VUID-vkCmdDraw-None-04008: States that vertex binding 0 is null handle, and must be bound.
		// Not entirely sure why we had added vertex bindings to this in the first place?
		// builder.setAttributeDescriptions( SimpleVertex::getAttributeDescriptions() );
		// builder.setBindingDescriptions( SimpleVertex::getBindingDescriptions() );

		builder.setVertexShader( Shader::loadVertex( "shaders/gui-compose.slang" ) );
		builder.setFragmentShader( Shader::loadFragment( "shaders/gui-compose.slang" ) );

		builder.addColorAttachment().setFormat( pickPresentFormat() ).finish();
		builder.addDepthAttachment();

		m_pipeline = builder.create();
		m_pipeline->setDebugName( "Gui Pipeline" );
	}

	CommandBuffer& GuiSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer.imgui_cb };

		m_pipeline->bind( command_buffer );

		// m_pipeline->bindDescriptor( command_buffer, 0, info.gui_input_descriptor );

		return command_buffer;
	}

	void GuiSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "GuiSystem::pass" );
		auto& command_buffer { setupSystem( info ) };

		command_buffer->draw( 3, 1, 0, 0 );
	}

} // namespace fgl::engine
