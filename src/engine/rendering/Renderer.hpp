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
		PhysicalDevice& m_phy_device;
		std::unique_ptr< SwapChain > m_swapchain;

		std::vector< vk::raii::CommandBuffer> m_command_buffer {};

		std::optional< TracyVkCtx > m_tracy_ctx { std::nullopt };

		void createCommandBuffers();
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

		vk::raii::CommandBuffer& getCurrentCommandbuffer()
		{
			assert( is_frame_started && "Cannot get command buffer while frame not in progress" );
			return m_command_buffer[ current_frame_idx ];
		}

		TracyVkCtx getCurrentTracyCTX() const
		{
#if TRACY_ENABLE
			assert( m_tracy_ctx.has_value() );
			return m_tracy_ctx.value();
#else
			return nullptr;
#endif
		}

		vk::raii::RenderPass& getSwapChainRenderPass() const { return m_swapchain->getRenderPass(); }

		float getAspectRatio() const { return m_swapchain->extentAspectRatio(); }

		vk::raii::CommandBuffer& beginFrame();
		void endFrame();
		void beginSwapchainRendererPass( vk::raii::CommandBuffer& buffer );
		void endSwapchainRendererPass( vk::raii::CommandBuffer& buffer );

		Renderer( Window& window, PhysicalDevice& phy_device );
		~Renderer();
		Renderer( Renderer&& other ) = delete;
		Renderer( const Renderer& other ) = delete;
		Renderer& operator=( const Renderer& other ) = delete;
	};

} // namespace fgl::engine