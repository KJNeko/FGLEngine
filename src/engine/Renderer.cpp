//
// Created by kj16609 on 11/28/23.
//

#include "Renderer.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "Device.hpp"
#include "SwapChain.hpp"
#include "Window.hpp"
#include "engine/debug_gui/DebugGUI.hpp"

//clang-format: off
#include <Tracy/tracy/TracyVulkan.hpp>

//clang-format: on

namespace fgl::engine
{

	Renderer::Renderer( Window& window, Device& device ) : m_window( window ), m_device( device )
	{
		recreateSwapchain();
		createCommandBuffers();

		debug::initDebugGUI();
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}

	void Renderer::createCommandBuffers()
	{
		m_command_buffer.resize( SwapChain::MAX_FRAMES_IN_FLIGHT );

		VkCommandBufferAllocateInfo alloc_info { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			                                     .commandPool = m_device.getCommandPool(),
			                                     .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			                                     .commandBufferCount =
			                                         static_cast< std::uint32_t >( m_command_buffer.size() ) };

		if ( vkAllocateCommandBuffers( m_device.device(), &alloc_info, m_command_buffer.data() ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to allocate command buffers" );

#ifdef TRACY_ENABLE
		for ( auto& cmd_buffer : m_command_buffer )
		{
			auto context =
				TracyVkContext( m_device.phyDevice(), m_device.device(), m_device.graphicsQueue(), cmd_buffer )
					m_tracy_ctx.emplace_back( context );
		}
#endif
	}

	void Renderer::recreateSwapchain()
	{
		auto extent { m_window.getExtent() };

		while ( extent.width == 0 || extent.height == 0 )
		{
			extent = m_window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle( m_device.device() );

		if ( m_swapchain == nullptr )
			m_swapchain = std::make_unique< SwapChain >( m_device, extent );
		else
		{
			std::shared_ptr< SwapChain > old_swap_chain { std::move( m_swapchain ) };
			m_swapchain = std::make_unique< SwapChain >( m_device, extent, old_swap_chain );

			if ( !old_swap_chain->compareSwapFormats( *m_swapchain.get() ) )
				throw std::runtime_error( "Swap chain image(or depth) format has changed!" );
		}
	}

	void Renderer::freeCommandBuffers()
	{
		if ( m_command_buffer.size() == 0 ) return;

		for ( auto& ctx : m_tracy_ctx ) TracyVkDestroy( ctx );

		vkFreeCommandBuffers(
			m_device.device(),
			m_device.getCommandPool(),
			static_cast< std::uint32_t >( m_command_buffer.size() ),
			m_command_buffer.data() );
	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert( !is_frame_started && "Cannot begin frame while frame is already in progress" );
		auto result { m_swapchain->acquireNextImage( &current_image_idx ) };

		if ( result == VK_ERROR_OUT_OF_DATE_KHR )
		{
			recreateSwapchain();
			return nullptr;
		}

		if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
			throw std::runtime_error( "Failed to acquire support chain image" );

		is_frame_started = true;
		auto command_buffer { getCurrentCommandbuffer() };

		VkCommandBufferBeginInfo begin_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};

		if ( vkBeginCommandBuffer( command_buffer, &begin_info ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to begin recording to command buffer" );

		TracyVkCollect( getCurrentTracyCTX(), command_buffer );

		return command_buffer;
	}

	void Renderer::endFrame()
	{
		assert( is_frame_started && "Cannot call end frame while frame is not in progress" );

		auto command_buffer { getCurrentCommandbuffer() };

		if ( vkEndCommandBuffer( command_buffer ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to end recording command buffer" );

		const auto result { m_swapchain->submitCommandBuffers( &command_buffer, &current_image_idx ) };

		if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized() )
		{
			m_window.resetWindowResizedFlag();
			recreateSwapchain();
		}
		else if ( result != VK_SUCCESS )
			throw std::runtime_error( "Failed to submit commmand buffer" );

		is_frame_started = false;
		current_frame_idx = ( current_frame_idx + 1 ) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::beginSwapchainRendererPass( VkCommandBuffer buffer )
	{
		assert( is_frame_started && "Cannot call beginSwapChainRenderPass if frame is not in progress" );
		assert(
			buffer == getCurrentCommandbuffer()
			&& "Cannot begin render pass on command buffer from a different frame" );

		std::array< VkClearValue, 2 > clear_values {};
		clear_values[ 0 ].color = { { 0.1f, 0.1f, 0.1f, 0.1f } };
		clear_values[ 1 ].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo render_pass_info { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			                                     .renderPass = m_swapchain->getRenderPass(),
			                                     .framebuffer = m_swapchain->getFrameBuffer( current_image_idx ),
			                                     .renderArea = { .offset = { 0, 0 },
			                                                     .extent = m_swapchain->getSwapChainExtent() },
			                                     .clearValueCount = static_cast< std::uint32_t >( clear_values.size() ),
			                                     .pClearValues = clear_values.data() };

		vkCmdBeginRenderPass( buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE );

		VkViewport viewport {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast< float >( m_swapchain->getSwapChainExtent().width ),
			.height = static_cast< float >( m_swapchain->getSwapChainExtent().height ),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};

		VkRect2D scissor { { 0, 0 }, m_swapchain->getSwapChainExtent() };
		vkCmdSetViewport( buffer, 0, 1, &viewport );
		vkCmdSetScissor( buffer, 0, 1, &scissor );
	}

	void Renderer::endSwapchainRendererPass( VkCommandBuffer buffer )
	{
		assert( is_frame_started && "Cannot call endSwapChainRenderPass if frame is not in progress" );
		assert(
			buffer == getCurrentCommandbuffer() && "Cannot end render pass on command buffer from a different frame" );

		vkCmdEndRenderPass( buffer );
	}
} // namespace fgl::engine
