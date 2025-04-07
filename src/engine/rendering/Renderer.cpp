//
// Created by kj16609 on 11/28/23.
//

#include "Renderer.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <array>
#include <iostream>
#include <stdexcept>

#include "PresentSwapChain.hpp"
#include "engine/Window.hpp"

//clang-format: off
#include <tracy/TracyVulkan.hpp>

#include "RenderingFormats.hpp"

//clang-format: on

namespace fgl::engine
{

	/*
	void Renderer::clearInputImage( vk::raii::CommandBuffer& command_buffer )
	{
		Image& image { getSwapChain().getInputImage( current_present_index ) };

		constexpr vk::ImageSubresourceRange range { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

		vk::ImageMemoryBarrier in_barrier {};
		in_barrier.oldLayout = vk::ImageLayout::eUndefined;
		in_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		in_barrier.image = image.getVkImage();
		in_barrier.subresourceRange = range;
		in_barrier.srcAccessMask = {};
		in_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			nullptr,
			nullptr,
			in_barrier );

		command_buffer.clearColorImage(
			image.getVkImage(),
			vk::ImageLayout::eTransferDstOptimal,
			vk::ClearColorValue( 0.0f, 0.0f, 0.0f, 0.0f ),
			range );

		in_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		in_barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		in_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		in_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{},
			nullptr,
			nullptr,
			in_barrier );
	}
	*/

	Renderer::Renderer( Window& window, PhysicalDevice& phy_device ) :
	  m_window( window ),
	  m_phy_device( phy_device ),
	  m_swapchain( std::make_unique< PresentSwapChain >( m_window.getExtent(), m_phy_device ) )
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
		alloc_info.commandBufferCount = constants::MAX_FRAMES_IN_FLIGHT;

		//TODO: Restore tracy functionality, It's wanting a command buffer.
		//TODO: Figure out if the command buffer it wants is just for setup or needs to be ran occasionally
		// m_tracy_ctx = createContext( m_phy_device, Device::getInstance(), m_command_buffer[ 0 ] );

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
			m_swapchain = std::make_unique< PresentSwapChain >( extent, m_phy_device );
		else
		{
			std::shared_ptr< PresentSwapChain > old_swap_chain { std::move( m_swapchain ) };
			m_swapchain = std::make_unique< PresentSwapChain >( extent, old_swap_chain );

			if ( !old_swap_chain->compareSwapFormats( *m_swapchain.get() ) )
				throw std::runtime_error( "Swap chain image(or depth) format has changed!" );
		}
	}

	std::optional< CommandBuffers > Renderer::beginFrame()
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

		auto command_buffers {
			Device::getInstance().getCmdBufferPool().getCommandBuffers( 4, CommandBufferHandle::Secondary )
		};

		vk::CommandBufferBeginInfo info {};
		info.pNext = VK_NULL_HANDLE;

		vk::CommandBufferInheritanceInfo inheritance_info {};
		inheritance_info.framebuffer = VK_NULL_HANDLE;
		inheritance_info.renderPass = VK_NULL_HANDLE;
		inheritance_info.subpass = 0;

		info.pInheritanceInfo = &inheritance_info;

		for ( auto& command_buffer : command_buffers )
		{
			command_buffer->begin( info );
		}

		CommandBuffers buffers { std::move( command_buffers ) };

		return buffers;
	}

	void Renderer::endFrame( CommandBuffers& buffers )
	{
		ZoneScopedN( "Ending frame" );
		assert( is_frame_started && "Cannot call end frame while frame is not in progress" );

		// auto& command_buffer { getCurrentCommandbuffer() };

		// Get a primary command buffer
		auto primary_cmd { Device::getInstance().getCmdBufferPool().getCommandBuffer( CommandBufferHandle::Primary ) };

		primary_cmd.setName( "Primary command buffer" );

		vk::CommandBufferBeginInfo begin_info {};
		begin_info.pNext = VK_NULL_HANDLE;
		begin_info.pInheritanceInfo = VK_NULL_HANDLE;
		primary_cmd->begin( begin_info );

		buffers.transfer_cb->end();
		buffers.transfer_cb.setName( "Transfer Commands" );
		buffers.render_cb->end();
		buffers.render_cb.setName( "Render Commands" );
		buffers.composition_cb->end();
		buffers.composition_cb.setName( "Composition Commands" );
		buffers.imgui_cb->end();
		buffers.imgui_cb.setName( "ImGui Commands" );

		// run all secondary command buffers
		primary_cmd->executeCommands(
			// { *buffers.transfer_cb, *buffers.render_cb, *buffers.composition_cb, *buffers.imgui_cb } );
			{ *buffers.transfer_cb, *buffers.render_cb, *buffers.composition_cb } );

		primary_cmd->pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, {} );

		primary_cmd->executeCommands( { *buffers.imgui_cb } );

		primary_cmd->end();

		const auto result { m_swapchain->submitCommandBuffers( primary_cmd, current_present_index ) };

		if ( result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR
		     || m_window.wasWindowResized() )
		{
			m_window.resetWindowResizedFlag();
			recreateSwapchain();
		}
		else if ( result != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to submit commmand buffer" );

		is_frame_started = false;
		current_frame_idx = static_cast< std::uint16_t >( ( current_frame_idx + 1 ) % constants::MAX_FRAMES_IN_FLIGHT );
	}

	void Renderer::setViewport( const CommandBuffer& buffer )
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( m_swapchain->getSwapChainExtent().width );
		viewport.height = static_cast< float >( m_swapchain->getSwapChainExtent().height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const std::vector< vk::Viewport > viewports { viewport };

		buffer->setViewport( 0, viewports );
	}

	void Renderer::setScissor( const CommandBuffer& buffer )
	{
		const vk::Rect2D scissor { { 0, 0 }, m_swapchain->getSwapChainExtent() };

		const std::vector< vk::Rect2D > scissors { scissor };

		buffer->setScissor( 0, scissors );
	}

	void Renderer::beginSwapchainRendererPass( CommandBuffer& buffer )
	{
		assert( is_frame_started && "Cannot call beginSwapChainRenderPass if frame is not in progress" );

		vk::RenderingInfo info { m_swapchain->getRenderingInfo( current_present_index ) };

		m_swapchain->transitionImages( buffer, PresentSwapChain::INITAL, current_present_index );

		buffer->beginRendering( info );

		setViewport( buffer );
		setScissor( buffer );
	}

	void Renderer::endSwapchainRendererPass( CommandBuffer& buffer )
	{
		assert( is_frame_started && "Cannot call endSwapChainRenderPass if frame is not in progress" );

		buffer->endRendering();

		m_swapchain->transitionImages( buffer, PresentSwapChain::FINAL, current_present_index );
	}
} // namespace fgl::engine
