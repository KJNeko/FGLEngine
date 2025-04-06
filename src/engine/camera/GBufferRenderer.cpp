//
// Created by kj16609 on 7/21/24.
//

#include "GBufferRenderer.hpp"

#include "Camera.hpp"
#include "GBufferSwapchain.hpp"
#include "engine/rendering/renderpass/RenderPass.hpp"

namespace fgl::engine
{
	class GBufferSwapchain;

	void GBufferRenderer::setViewport( const CommandBuffer& command_buffer, const vk::Extent2D extent )
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( extent.width );
		viewport.height = static_cast< float >( extent.height );

		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const std::vector< vk::Viewport > viewports { viewport };

		command_buffer->setViewport( 0, viewports );
	}

	void GBufferRenderer::setScissor( const CommandBuffer& command_buffer, const vk::Extent2D extent )
	{
		const vk::Rect2D scissor { { 0, 0 }, extent };

		const std::vector< vk::Rect2D > scissors { scissor };

		command_buffer->setScissor( 0, scissors );
	}

	void GBufferRenderer::
		beginRenderPass( const CommandBuffer& command_buffer, GBufferSwapchain& swapchain, const FrameIndex index )
	{
		const vk::RenderingInfo info { swapchain.getRenderingInfo( index ) };

		command_buffer->beginRendering( info );

		setViewport( command_buffer, swapchain.getExtent() );
		setScissor( command_buffer, swapchain.getExtent() );
	}

	void GBufferRenderer::endRenderPass( const CommandBuffer& command_buffer )
	{
		command_buffer->endRendering();
	}

	void GBufferRenderer::pass( FrameInfo& frame_info, GBufferSwapchain& camera_swapchain )
	{
		ZoneScopedN( "CameraRenderer::pass" );

		m_culling_system.pass( frame_info );

		auto& command_buffer { frame_info.command_buffer.render_cb };

		camera_swapchain.transitionImages( command_buffer, GBufferSwapchain::INITAL, frame_info.in_flight_idx );

		beginRenderPass( command_buffer, camera_swapchain, frame_info.in_flight_idx );

		//m_terrain_system.pass( frame_info );

		m_entity_renderer.pass( frame_info );
		m_line_drawer.pass( frame_info );

		endRenderPass( command_buffer );

		camera_swapchain.transitionImages( command_buffer, GBufferSwapchain::FINAL, frame_info.in_flight_idx );

		m_compositor.composite( command_buffer, *frame_info.camera, frame_info.in_flight_idx );
	}

} // namespace fgl::engine
