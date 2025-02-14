//
// Created by kj16609 on 11/28/23.
//

#pragma once

// clang-format off
#include <vulkan/vulkan.hpp>
#include <tracy/TracyVulkan.hpp>
// clang-format on

#include <cassert>
#include <memory>

#include "CommandBuffers.hpp"
#include "PresentSwapChain.hpp"

namespace fgl::engine
{

	class Renderer
	{
		Window& m_window;
		PhysicalDevice& m_phy_device;
		std::unique_ptr< PresentSwapChain > m_swapchain;

		TracyVkCtx m_tracy_ctx { nullptr };

		PresentIndex current_present_index { std::numeric_limits< PresentIndex >::max() };
		FrameIndex current_frame_idx { 0 };
		bool is_frame_started { false };

		void createCommandBuffers();
		void recreateSwapchain();

	  public:

		FrameIndex getFrameIndex() const
		{
			assert( is_frame_started && "Cannot get frame index while frame not in progress" );
			return current_frame_idx;
		}

		PresentIndex getPresentIndex() const
		{
			assert( current_present_index != std::numeric_limits< PresentIndex >::max() );
			return current_present_index;
		}

		bool isFrameInProgress() const { return is_frame_started; }

		TracyVkCtx getCurrentTracyCTX() const { return m_tracy_ctx; }

		float getAspectRatio() const { return m_swapchain->extentAspectRatio(); }

		// vk::raii::CommandBuffer& beginFrame();
		std::optional< CommandBuffers > beginFrame();

		void endFrame( CommandBuffers& buffers );

		void setViewport( const CommandBuffer& buffer );
		void setScissor( const CommandBuffer& buffer );

		void beginSwapchainRendererPass( CommandBuffer& buffer );
		void endSwapchainRendererPass( CommandBuffer& buffer );

		PresentSwapChain& getSwapChain() { return *m_swapchain; }

		// void clearInputImage( vk::raii::CommandBuffer& command_buffer );

		Renderer( Window& window, PhysicalDevice& phy_device );
		~Renderer();
		Renderer( Renderer&& other ) = delete;
		Renderer( const Renderer& other ) = delete;
		Renderer& operator=( const Renderer& other ) = delete;
	};

} // namespace fgl::engine