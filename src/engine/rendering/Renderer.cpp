//
// Created by kj16609 on 11/28/23.
//

#include "Renderer.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <array>
#include <iostream>
#include <stdexcept>

#include "SwapChain.hpp"
#include "engine/Window.hpp"

//clang-format: off
#include <tracy/TracyVulkan.hpp>

//clang-format: on

namespace fgl::engine
{

	void Renderer::clearInputImage( vk::raii::CommandBuffer& command_buffer )
	{
		auto& image { getSwapChain().getInputImage( current_present_index ) };

		vk::ImageSubresourceRange range {};
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.layerCount = 1;
		range.baseArrayLayer = 0;
		range.levelCount = 1;
		range.baseMipLevel = 0;

		/*
		command_buffer.clearColorImage(
			image.getVkImage(),
			vk::ImageLayout::eTransferDstOptimal,
			vk::ClearColorValue( 0.0f, 0.0f, 0.0f, 0.0f ),
			{ range } );
		*/

		// Transition the image back to readOnly
		vk::ImageMemoryBarrier barrier {};
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.image = image.getVkImage();
		barrier.subresourceRange = range;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { barrier } );
	}

	Renderer::Renderer( Window& window, PhysicalDevice& phy_device ) :
	  m_window( window ),
	  m_phy_device( phy_device ),
	  m_swapchain( std::make_unique< SwapChain >( m_window.getExtent(), m_phy_device ) )
	{
		recreateSwapchain();
		createCommandBuffers();
	}

	Renderer::~Renderer()
	{}

	TracyVkCtx createContext(
		[[maybe_unused]] PhysicalDevice& physical_device,
		[[maybe_unused]] Device& device,
		[[maybe_unused]] vk::raii::CommandBuffer& cmd_buffer )
	{
#if ENABLE_CALIBRATED_PROFILING

		// The calibrated context wants the calibration extention and two function pointers.
		auto getPhysicalDeviceClibrateableTimeDomains {
			reinterpret_cast< PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT >(
				device->getProcAddr( "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR" ) )
		};

		auto getCalibratedTimestamps {
			reinterpret_cast< PFN_vkGetCalibratedTimestampsEXT >( device
			                                                          ->getProcAddr( "vkGetCalibratedTimestampsKHR" ) )
		};

		if ( getPhysicalDeviceClibrateableTimeDomains == nullptr )
			throw std::runtime_error( "Failed to get vkGetPhysicalDeviceCalibratableTimeDomainsEXT" );

		if ( getCalibratedTimestamps == nullptr )
			throw std::runtime_error( "Failed to get vkGetCalibratedTimestampsEXT" );

		return TracyVkContextCalibrated(
			*physical_device,
			*device,
			*device.graphicsQueue(),
			*cmd_buffer,
			getPhysicalDeviceClibrateableTimeDomains,
			getCalibratedTimestamps );
#else
		return TracyVkContext( *physical_device, *device, *device.graphicsQueue(), *cmd_buffer );
#endif
	}

	void Renderer::createCommandBuffers()
	{
		vk::CommandBufferAllocateInfo alloc_info {};
		alloc_info.pNext = VK_NULL_HANDLE;
		alloc_info.commandPool = Device::getInstance().getCommandPool();
		alloc_info.level = vk::CommandBufferLevel::ePrimary;
		alloc_info.commandBufferCount = SwapChain::MAX_FRAMES_IN_FLIGHT;

		m_command_buffer = Device::getInstance().device().allocateCommandBuffers( alloc_info );

		m_tracy_ctx = createContext( m_phy_device, Device::getInstance(), m_command_buffer[ 0 ] );

		/*
		m_tracy_ctx = TracyVkContextCalibrated(
			phy_dev,
			dev,
			Device::getInstance().graphicsQueue(),
			m_command_buffer[ 0 ],
			VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
			VULKAN_HPP_DEFAULT_DISPATCHER.vkGetCalibratedTimestampsEXT );
		*/

		alloc_info.level = vk::CommandBufferLevel::eSecondary;

		m_gui_command_buffer = Device::getInstance()->allocateCommandBuffers( alloc_info );
	}

	void Renderer::recreateSwapchain()
	{
		ZoneScoped;
		std::cout << "Rebuilding swap chain" << std::endl;
		auto extent { m_window.getExtent() };

		while ( extent.width == 0 || extent.height == 0 )
		{
			extent = m_window.getExtent();
			glfwWaitEvents();
		}

		Device::getInstance().device().waitIdle();

		if ( m_swapchain == nullptr )
			m_swapchain = std::make_unique< SwapChain >( extent, m_phy_device );
		else
		{
			std::shared_ptr< SwapChain > old_swap_chain { std::move( m_swapchain ) };
			m_swapchain = std::make_unique< SwapChain >( extent, old_swap_chain );

			if ( !old_swap_chain->compareSwapFormats( *m_swapchain.get() ) )
				throw std::runtime_error( "Swap chain image(or depth) format has changed!" );
		}
	}

	vk::raii::CommandBuffer& Renderer::beginFrame()
	{
		assert( !is_frame_started && "Cannot begin frame while frame is already in progress" );
		auto [ result, present_index ] = m_swapchain->acquireNextImage();
		current_present_index = present_index;

		if ( result == vk::Result::eErrorOutOfDateKHR )
		{
			recreateSwapchain();
		}

		if ( result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR )
			throw std::runtime_error( "Failed to acquire swap chain image" );

		is_frame_started = true;
		auto& command_buffer { getCurrentCommandbuffer() };

		vk::CommandBufferBeginInfo begin_info {};
		begin_info.pNext = VK_NULL_HANDLE;
		//begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		begin_info.pInheritanceInfo = VK_NULL_HANDLE;

		command_buffer.begin( begin_info );

		return command_buffer;
	}

	void Renderer::endFrame()
	{
		ZoneScopedN( "Ending frame" );
		assert( is_frame_started && "Cannot call end frame while frame is not in progress" );

		auto& command_buffer { getCurrentCommandbuffer() };

		command_buffer.end();

		const auto result { m_swapchain->submitCommandBuffers( command_buffer, current_present_index ) };

		if ( result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR
		     || m_window.wasWindowResized() )
		{
			m_window.resetWindowResizedFlag();
			recreateSwapchain();
		}
		else if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to submit commmand buffer" );

		is_frame_started = false;
		current_frame_idx = static_cast< std::uint16_t >( ( current_frame_idx + 1 ) % SwapChain::MAX_FRAMES_IN_FLIGHT );
	}

	void Renderer::setViewport( const vk::raii::CommandBuffer& buffer )
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( m_swapchain->getSwapChainExtent().width );
		viewport.height = static_cast< float >( m_swapchain->getSwapChainExtent().height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const std::vector< vk::Viewport > viewports { viewport };

		buffer.setViewport( 0, viewports );
	}

	void Renderer::setScissor( const vk::raii::CommandBuffer& buffer )
	{
		const vk::Rect2D scissor { { 0, 0 }, m_swapchain->getSwapChainExtent() };

		const std::vector< vk::Rect2D > scissors { scissor };

		buffer.setScissor( 0, scissors );
	}

	void Renderer::beginSwapchainRendererPass( vk::raii::CommandBuffer& buffer )
	{
		assert( is_frame_started && "Cannot call beginSwapChainRenderPass if frame is not in progress" );

		vk::RenderPassBeginInfo render_pass_info {};
		render_pass_info.pNext = VK_NULL_HANDLE;
		render_pass_info.renderPass = m_swapchain->getRenderPass();
		render_pass_info.framebuffer = m_swapchain->getFrameBuffer( current_present_index );
		render_pass_info.renderArea = { .offset = { 0, 0 }, .extent = m_swapchain->getSwapChainExtent() };

		render_pass_info.setClearValues( m_swapchain->getClearValues() );

		buffer.beginRenderPass( render_pass_info, vk::SubpassContents::eInline );

		setViewport( buffer );
		setScissor( buffer );
	}

	void Renderer::endSwapchainRendererPass( vk::raii::CommandBuffer& buffer )
	{
		assert( is_frame_started && "Cannot call endSwapChainRenderPass if frame is not in progress" );

		buffer.endRenderPass();
	}
} // namespace fgl::engine
