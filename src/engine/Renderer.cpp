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

//clang-format: off
#include <tracy/TracyVulkan.hpp>

//clang-format: on

namespace fgl::engine
{

	Renderer::Renderer( Window& window ) : m_window( window )
	{
		recreateSwapchain();
		createCommandBuffers();
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}

	void Renderer::createCommandBuffers()
	{
		m_command_buffer.resize( SwapChain::MAX_FRAMES_IN_FLIGHT );

		vk::CommandBufferAllocateInfo alloc_info {};
		alloc_info.pNext = VK_NULL_HANDLE;
		alloc_info.commandPool = Device::getInstance().getCommandPool();
		alloc_info.level = vk::CommandBufferLevel::ePrimary;
		alloc_info.commandBufferCount = static_cast< std::uint32_t >( m_command_buffer.size() );

		if ( Device::getInstance().device().allocateCommandBuffers( &alloc_info, m_command_buffer.data() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to allocate command buffers" );

#ifdef TRACY_ENABLE
#if TRACY_ENABLE
		m_tracy_ctx.resize( SwapChain::MAX_FRAMES_IN_FLIGHT );

		for ( int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			VkPhysicalDevice phy_dev { Device::getInstance().phyDevice() };
			VkDevice dev { Device::getInstance().device() };

			m_tracy_ctx[ i ] =
				TracyVkContext( phy_dev, dev, Device::getInstance().graphicsQueue(), m_command_buffer[ i ] );
		}
#endif
#endif
	}

	void Renderer::recreateSwapchain()
	{
		std::cout << "Rebuilding swap chain" << std::endl;
		auto extent { m_window.getExtent() };

		while ( extent.width == 0 || extent.height == 0 )
		{
			extent = m_window.getExtent();
			glfwWaitEvents();
		}

		Device::getInstance().device().waitIdle();

		if ( m_swapchain == nullptr )
			m_swapchain = std::make_unique< SwapChain >( extent );
		else
		{
			std::shared_ptr< SwapChain > old_swap_chain { std::move( m_swapchain ) };
			m_swapchain = std::make_unique< SwapChain >( extent, old_swap_chain );

			if ( !old_swap_chain->compareSwapFormats( *m_swapchain.get() ) )
				throw std::runtime_error( "Swap chain image(or depth) format has changed!" );
		}
	}

	void Renderer::freeCommandBuffers()
	{
		if ( m_command_buffer.size() == 0 ) return;

		Device::getInstance().device().freeCommandBuffers(
			Device::getInstance().getCommandPool(),
			static_cast< std::uint32_t >( m_command_buffer.size() ),
			m_command_buffer.data() );
	}

	vk::CommandBuffer Renderer::beginFrame()
	{
		assert( !is_frame_started && "Cannot begin frame while frame is already in progress" );
		vk::Result result { m_swapchain->acquireNextImage( &current_image_idx ) };

		if ( result == vk::Result::eErrorOutOfDateKHR )
		{
			recreateSwapchain();
			return nullptr;
		}

		if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
			throw std::runtime_error( "Failed to acquire swap chain image" );

		is_frame_started = true;
		auto command_buffer { getCurrentCommandbuffer() };

		vk::CommandBufferBeginInfo begin_info {};
		begin_info.pNext = VK_NULL_HANDLE;
		begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		begin_info.pInheritanceInfo = VK_NULL_HANDLE;

		command_buffer.begin( begin_info );

		return command_buffer;
	}

	void Renderer::endFrame()
	{
		assert( is_frame_started && "Cannot call end frame while frame is not in progress" );

		auto command_buffer { getCurrentCommandbuffer() };

		if ( vkEndCommandBuffer( command_buffer ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to end recording command buffer" );

		const auto result { m_swapchain->submitCommandBuffers( &command_buffer, &current_image_idx ) };

		if ( result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR
		     || m_window.wasWindowResized() )
		{
			m_window.resetWindowResizedFlag();
			recreateSwapchain();
		}
		else if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to submit commmand buffer" );

		is_frame_started = false;
		current_frame_idx = static_cast< std::uint8_t >( ( current_frame_idx + 1 ) % SwapChain::MAX_FRAMES_IN_FLIGHT );
	}

	void Renderer::beginSwapchainRendererPass( vk::CommandBuffer buffer )
	{
		assert( is_frame_started && "Cannot call beginSwapChainRenderPass if frame is not in progress" );
		assert(
			buffer == getCurrentCommandbuffer()
			&& "Cannot begin render pass on command buffer from a different frame" );

		//TODO: Attach this stuff into the swapchain creation via attachments and request it again here
		//std::array< vk::ClearValue, 5 > clear_values {};

		std::vector< vk::ClearValue > clear_values { m_swapchain->getClearValues() };

		vk::RenderPassBeginInfo render_pass_info {};
		render_pass_info.pNext = VK_NULL_HANDLE;
		render_pass_info.renderPass = m_swapchain->getRenderPass();
		render_pass_info.framebuffer = m_swapchain->getFrameBuffer( static_cast< int >( current_image_idx ) );
		render_pass_info.renderArea = { .offset = { 0, 0 }, .extent = m_swapchain->getSwapChainExtent() };
		render_pass_info.clearValueCount = static_cast< std::uint32_t >( clear_values.size() );
		render_pass_info.pClearValues = clear_values.data();

		buffer.beginRenderPass( &render_pass_info, vk::SubpassContents::eInline );

		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( m_swapchain->getSwapChainExtent().width );
		viewport.height = static_cast< float >( m_swapchain->getSwapChainExtent().height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor { { 0, 0 }, m_swapchain->getSwapChainExtent() };
		buffer.setViewport( 0, 1, &viewport );
		buffer.setScissor( 0, 1, &scissor );
	}

	void Renderer::nextPass()
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( m_swapchain->getSwapChainExtent().width );
		viewport.height = static_cast< float >( m_swapchain->getSwapChainExtent().height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor { { 0, 0 }, m_swapchain->getSwapChainExtent() };

		auto buffer = getCurrentCommandbuffer();
		buffer.nextSubpass( vk::SubpassContents::eInline );
		buffer.setViewport( 0, 1, &viewport );
		buffer.setScissor( 0, 1, &scissor );
	}

	void Renderer::endSwapchainRendererPass( vk::CommandBuffer buffer )
	{
		assert( is_frame_started && "Cannot call endSwapChainRenderPass if frame is not in progress" );
		assert(
			buffer == getCurrentCommandbuffer() && "Cannot end render pass on command buffer from a different frame" );

		vkCmdEndRenderPass( buffer );
	}
} // namespace fgl::engine
