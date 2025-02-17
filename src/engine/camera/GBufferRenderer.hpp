//
// Created by kj16609 on 7/21/24.
//

#pragma once

#include "GBufferCompositor.hpp"
#include "engine/systems/prerender/CullingSystem.hpp"
#include "engine/systems/render/EntityRendererSystem.hpp"
#include "engine/systems/render/LineDrawer.hpp"

namespace fgl::engine
{

	class GBufferRenderer
	{
		GBufferCompositor m_compositor {};

		CullingSystem m_culling_system {};

		// SubPass 0
		//TerrainSystem m_terrain_system { Device::getInstance(), m_renderpass };
		EntityRendererSystem m_entity_renderer {};
		LineDrawer m_line_drawer {};

		// SubPass 1
		// CompositionSystem m_composition_system {};

		void beginRenderPass( const CommandBuffer& command_buffer, GBufferSwapchain& swapchain, FrameIndex index );

		void setViewport( const CommandBuffer& command_buffer, vk::Extent2D extent );
		void setScissor( const CommandBuffer& command_buffer, vk::Extent2D extent );

		void endRenderPass( const CommandBuffer& command_buffer );

	  public:

		void pass( FrameInfo& frame_info, GBufferSwapchain& camera_swapchain );

		GBufferRenderer() = default;
	};

} // namespace fgl::engine
