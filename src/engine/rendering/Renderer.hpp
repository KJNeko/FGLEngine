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
#include "engine/Window.hpp"

//clang-format: off
#include <tracy/TracyVulkan.hpp>

//clang-format: on

namespace fgl::engine
{

	class Renderer
	{
		Window& m_window;
		std::unique_ptr< SwapChain > m_swapchain { std::make_unique< SwapChain >( m_window.getExtent() ) };

		std::vector< vk::CommandBuffer > m_command_buffer {};

		std::vector< TracyVkCtx > m_tracy_ctx {};

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapchain();

		uint32_t current_image_idx { std::numeric_limits< std::uint32_t >::max() };
		std::uint16_t current_frame_idx { 0 };
		bool is_frame_started { false };

	  public:

		DescriptorSet& getGBufferDescriptor( std::uint16_t frame_idx ) const
		{
			return m_swapchain->getGBufferDescriptor( frame_idx );
		}

		std::uint16_t getFrameIndex() const
		{
			assert( is_frame_started && "Cannot get frame index while frame not in progress" );
			return current_frame_idx;
		}

		bool isFrameInProgress() const { return is_frame_started; }

		vk::CommandBuffer& getCurrentCommandbuffer()
		{
			assert( is_frame_started && "Cannot get command buffer while frame not in progress" );
			return m_command_buffer[ current_frame_idx ];
		}

		TracyVkCtx getCurrentTracyCTX() const
		{
#if TRACY_ENABLE
			return m_tracy_ctx[ current_frame_idx ];
#else
			return nullptr;
#endif
		}

		vk::RenderPass getSwapChainRenderPass() const { return m_swapchain->getRenderPass(); }

		float getAspectRatio() const { return m_swapchain->extentAspectRatio(); }

		vk::CommandBuffer beginFrame();
		void endFrame();
		void beginSwapchainRendererPass( vk::CommandBuffer buffer );
		void endSwapchainRendererPass( vk::CommandBuffer buffer );

		Renderer( Window& window );
		~Renderer();
		Renderer( Renderer&& other ) = delete;
		Renderer( const Renderer& other ) = delete;
		Renderer& operator=( const Renderer& other ) = delete;
	};

} // namespace fgl::engine