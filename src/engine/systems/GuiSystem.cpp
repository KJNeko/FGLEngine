//
// Created by kj16609 on 6/23/24.
//

#include "GuiSystem.hpp"

#include "engine/FrameInfo.hpp"

namespace fgl::engine
{

	GuiSystem::GuiSystem( Device& device, vk::raii::RenderPass& render_pass )
	{
		PipelineConfigInfo info { render_pass };
		PipelineConfigInfo::addColorAttachmentConfig( info );
		PipelineConfigInfo::disableVertexInput( info );
		PipelineConfigInfo::disableCulling( info );
		info.subpass = 0;

		m_pipeline = std::make_unique< Pipeline >( device, std::move( info ) );
		m_pipeline->setDebugName( "Gui Pipeline" );
	}

	vk::raii::CommandBuffer& GuiSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };

		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, 0, info.gui_input_descriptor );

		return command_buffer;
	}

	void GuiSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "GuiSystem::pass" );
		auto& command_buffer { setupSystem( info ) };

		command_buffer.draw( 3, 1, 0, 0 );

		//Handle GUI
		// gui::drawMainGUI( info );
	}

} // namespace fgl::engine
