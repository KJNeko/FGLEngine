//
// Created by kj16609 on 2/28/25.
//
#include "ShadowRenderer.hpp"

#include "FrameInfo.hpp"

namespace fgl::engine
{
	CommandBuffer& ShadowRenderer::setupSystem( const FrameInfo& info )
	{}

	void ShadowRenderer::pass( FrameInfo& info )
	{
		// Render any shadowmaps attach to the camera
		auto& command_buffer { setupSystem( info ) };

		//TODO: Implement object culling for shadowmaps
		// if ( draw_commands.empty() ) return;

		m_pipeline->bind( command_buffer );
	}

	ShadowRenderer::ShadowRenderer()
	{
		PipelineBuilder builder { 0 };

		builder.setVertexShader( Shader::loadVertex( "shaders/shadowmap.slang" ) );

		m_pipeline = builder.create();
		m_pipeline->setDebugName( "Shadow map pipeline" );
	}

} // namespace fgl::engine