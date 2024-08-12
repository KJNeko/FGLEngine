//
// Created by kj16609 on 7/21/24.
//

#pragma once

#include "CameraSwapchain.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/systems/CompositionSystem.hpp"
#include "engine/systems/CullingSystem.hpp"
#include "engine/systems/EntityRendererSystem.hpp"
#include "engine/systems/LineDrawer.hpp"

namespace fgl::engine
{

	class CameraRenderer
	{
		vk::raii::RenderPass m_renderpass;

		static vk::raii::RenderPass createRenderPass();
		void setViewport( const vk::raii::CommandBuffer& command_buffer, vk::Extent2D extent );
		void setScissor( const vk::raii::CommandBuffer& command_buffer, vk::Extent2D extent );

		CullingSystem m_culling_system {};

		// SubPass 0
		//TerrainSystem m_terrain_system { Device::getInstance(), m_renderpass };
		EntityRendererSystem m_entity_renderer { Device::getInstance(), m_renderpass };
		LineDrawer m_line_drawer { Device::getInstance(), m_renderpass };

		// SubPass 1
		CompositionSystem m_composition_system { Device::getInstance(), m_renderpass };

		void beginRenderPass(
			const vk::raii::CommandBuffer& command_buffer, CameraSwapchain& swapchain, const FrameIndex index );

		void endRenderPass( const vk::raii::CommandBuffer& command_buffer );

	  public:

		void pass( FrameInfo& frame_info, CameraSwapchain& camera_swapchain );

		vk::raii::RenderPass& getRenderpass();

		CameraRenderer() : m_renderpass( createRenderPass() ) {}
	};

} // namespace fgl::engine
