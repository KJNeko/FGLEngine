//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <cassert>
#include <memory>

#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"

//clang-format: off
#include <Tracy/tracy/TracyVulkan.hpp>

//clang-format: on

namespace fgl::engine
{

	class Renderer
	{
		Window& m_window;
		Device& m_device;
		std::unique_ptr< SwapChain > m_swapchain { std::make_unique< SwapChain >( m_device, m_window.getExtent() ) };

		std::vector< VkCommandBuffer > m_command_buffer;

		std::vector< TracyVkCtx > m_tracy_ctx;

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();

		uint32_t current_image_idx {};
		int current_frame_idx { 0 };
		bool is_frame_started { false };

	  public:

		int getFrameIndex() const
		{
			assert( is_frame_started && "Cannot get frame index while frame not in progress" );
			return current_frame_idx;
		}

		bool isFrameInProgress() const { return is_frame_started; }

		VkCommandBuffer getCurrentCommandbuffer() const
		{
			assert( is_frame_started && "Cannot get command buffer while frame not in progress" );
			return m_command_buffer[ current_frame_idx ];
		}

		TracyVkCtx getCurrentTracyCTX() const { return m_tracy_ctx[ current_image_idx ]; }

		VkRenderPass getSwapChainRenderPass() const { return m_swapchain->getRenderPass(); }

		float getAspectRatio() const { return m_swapchain->extentAspectRatio(); }

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapchainRendererPass( VkCommandBuffer buffer );
		void endSwapchainRendererPass( VkCommandBuffer buffer );

		Renderer( Window& window, Device& device );
		~Renderer();
		Renderer( Renderer&& other ) = delete;
		Renderer( const Renderer& other ) = delete;
		Renderer& operator=( const Renderer& other ) = delete;
	};

} // namespace fgl::engine