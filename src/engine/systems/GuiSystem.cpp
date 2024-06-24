//
// Created by kj16609 on 6/23/24.
//

#include "GuiSystem.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/gui/core.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{

	GuiSystem::GuiSystem( Device& device, vk::raii::RenderPass& render_pass )
	{
		PipelineConfigInfo info { render_pass };
		PipelineConfigInfo::addColorAttachmentConfig( info );
		PipelineConfigInfo::disableVertexInput( info );
		PipelineConfigInfo::disableCulling( info );
		info.subpass = 2;

		m_pipeline = std::make_unique< Pipeline >( device, std::move( info ) );
		m_pipeline->setDebugName( "Gui Pipeline" );
	}

	vk::raii::CommandBuffer& GuiSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.gui_command_buffer };

		m_pipeline
			->bindDescriptor( command_buffer, GBufferCompositeDescriptorSet::m_set_idx, info.gbuffer_composite_set );

		m_pipeline->bind( command_buffer );

		return command_buffer;
	}

	void GuiSystem::pass( FrameInfo& info )
	{
		auto& command_buffer { setupSystem( info ) };

		command_buffer.draw( 3, 1, 0, 0 );

		//Handle GUI
		gui::drawMainGUI( info );

		command_buffer.end();

		info.command_buffer.nextSubpass( vk::SubpassContents::eSecondaryCommandBuffers );
		info.command_buffer.executeCommands( { info.gui_command_buffer } );
	}

} // namespace fgl::engine
