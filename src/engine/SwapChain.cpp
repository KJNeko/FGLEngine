#include "SwapChain.hpp"

#include "Attachment.hpp"
#include "RenderPass.hpp"
#include "Subpass.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace fgl::engine
{

	SwapChain::SwapChain( vk::Extent2D extent ) : windowExtent( extent )
	{
		init();
	}

	SwapChain::SwapChain( vk::Extent2D extent, std::shared_ptr< SwapChain > previous ) :
	  windowExtent( extent ),
	  old_swap_chain( previous )
	{
		init();
		old_swap_chain.reset();
	}

	void SwapChain::init()
	{
		createSwapChain();
		createRenderPass();
		createFramebuffers();
		createSyncObjects();
	}

	SwapChain::~SwapChain()
	{
		if ( swapChain != nullptr )
		{
			vkDestroySwapchainKHR( Device::getInstance().device(), swapChain, nullptr );
			swapChain = nullptr;
		}

		for ( auto framebuffer : m_swap_chain_buffers )
		{
			vkDestroyFramebuffer( Device::getInstance().device(), framebuffer, nullptr );
		}

		vkDestroyRenderPass( Device::getInstance().device(), m_render_pass, nullptr );

		// cleanup synchronization objects
		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			vkDestroySemaphore( Device::getInstance().device(), renderFinishedSemaphores[ i ], nullptr );
			vkDestroySemaphore( Device::getInstance().device(), imageAvailableSemaphores[ i ], nullptr );
			vkDestroyFence( Device::getInstance().device(), inFlightFences[ i ], nullptr );
		}
	}

	vk::Result SwapChain::acquireNextImage( uint32_t* imageIndex )
	{
		if ( Device::getInstance()
		         .device()
		         .waitForFences( 1, &inFlightFences[ currentFrame ], VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		vk::Result result { Device::getInstance().device().acquireNextImageKHR(
			swapChain,
			std::numeric_limits< uint64_t >::max(),
			imageAvailableSemaphores[ currentFrame ], // must be a not signaled semaphore
			VK_NULL_HANDLE,
			imageIndex ) };

		return result;
	}

	vk::Result SwapChain::submitCommandBuffers( const vk::CommandBuffer* buffers, uint32_t* imageIndex )
	{
		if ( imagesInFlight[ *imageIndex ] != VK_NULL_HANDLE )
		{
			if ( Device::getInstance().device().waitForFences(
					 1, &imagesInFlight[ *imageIndex ], VK_TRUE, std::numeric_limits< uint64_t >::max() )
			     != vk::Result::eSuccess )
				throw std::runtime_error( "failed to wait for fences!" );
		}
		imagesInFlight[ *imageIndex ] = inFlightFences[ currentFrame ];

		vk::SubmitInfo submitInfo {};

		vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[ currentFrame ] };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[ currentFrame ] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if ( Device::getInstance().device().resetFences( 1, &inFlightFences[ currentFrame ] ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to reset fences!" );

		if ( Device::getInstance().graphicsQueue().submit( 1, &submitInfo, inFlightFences[ currentFrame ] )
		     != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to submit draw command buffer!" );
		}

		vk::PresentInfoKHR presentInfo = {};

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		vk::SwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = imageIndex;

		if ( auto present_result = Device::getInstance().presentQueue().presentKHR( &presentInfo );
		     present_result != vk::Result::eSuccess )
		{
			if ( present_result == vk::Result::eSuboptimalKHR ) return vk::Result::eSuboptimalKHR;

			throw std::runtime_error( "failed to present swap chain image!" );
		}

		currentFrame = ( currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;

		return vk::Result::eSuccess;
	}

	void SwapChain::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport { Device::getInstance().getSwapChainSupport() };

		vk::SurfaceFormatKHR surfaceFormat { chooseSwapSurfaceFormat( swapChainSupport.formats ) };
		vk::PresentModeKHR presentMode { chooseSwapPresentMode( swapChainSupport.presentModes ) };
		vk::Extent2D extent { chooseSwapExtent( swapChainSupport.capabilities ) };

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if ( swapChainSupport.capabilities.maxImageCount > 0
		     && imageCount > swapChainSupport.capabilities.maxImageCount )
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo = {};
		createInfo.surface = Device::getInstance().surface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		QueueFamilyIndices indices = Device::getInstance().findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

		if ( indices.graphicsFamily != indices.presentFamily )
		{
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = old_swap_chain == nullptr ? VK_NULL_HANDLE : old_swap_chain->swapChain;

		if ( Device::getInstance().device().createSwapchainKHR( &createInfo, nullptr, &swapChain )
		     != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create swap chain!" );
		}

		std::vector< vk::Image > swap_chain_images {
			Device::getInstance().device().getSwapchainImagesKHR( swapChain )
		};

		for ( std::uint64_t i = 0; i < swap_chain_images.size(); i++ )
		{
			m_swap_chain_images
				.emplace_back( extent, surfaceFormat.format, swap_chain_images[ i ], createInfo.imageUsage );
		}

		m_swap_chain_format = surfaceFormat.format;
		m_swap_chain_extent = extent;
	}

	void SwapChain::createRenderPass()
	{
		//Present attachment
		ColoredPresentAttachment colorAttachment { getSwapChainImageFormat() };

		for ( int i = 0; i < imageCount(); ++i )
		{
			m_swap_chain_images[ i ].setName( "SwapChainImage: " + std::to_string( i ) );
		}

		colorAttachment.linkImages( m_swap_chain_images );

		// G-Buffer
		ColorAttachment g_buffer_position { vk::Format::eR16G16B16A16Sfloat };
		ColorAttachment g_buffer_normal { vk::Format::eR16G16B16A16Sfloat };
		ColorAttachment g_buffer_albedo { vk::Format::eR8G8B8A8Unorm };

		g_buffer_position.createResources( imageCount(), getSwapChainExtent() );
		g_buffer_normal.createResources( imageCount(), getSwapChainExtent() );
		g_buffer_albedo.createResources( imageCount(), getSwapChainExtent() );

		g_buffer_position.setClear( vk::ClearColorValue( std::array< float, 4 > { 0.0f, 0.0f, 0.0f, 0.0f } ) );
		g_buffer_normal.setClear( vk::ClearColorValue( std::array< float, 4 > { 0.0f, 0.0f, 0.0f, 0.0f } ) );
		g_buffer_albedo.setClear( vk::ClearColorValue( std::array< float, 4 > { 0.0f, 0.0f, 0.0f, 0.0f } ) );

		for ( int i = 0; i < imageCount(); ++i )
		{
			g_buffer_position.m_attachment_resources.m_images[ i ]
				->setName( "GBufferPosition: " + std::to_string( i ) );
		}

		for ( int i = 0; i < imageCount(); ++i )
		{
			g_buffer_normal.m_attachment_resources.m_images[ i ]->setName( "GBufferNormal: " + std::to_string( i ) );
		}
		for ( int i = 0; i < imageCount(); ++i )
		{
			g_buffer_albedo.m_attachment_resources.m_images[ i ]->setName( "GBufferAlbedo: " + std::to_string( i ) );
		}

		RenderPass render_pass {};

		DepthAttachment depthAttachment { findDepthFormat() };
		depthAttachment.createResources( imageCount(), getSwapChainExtent() );
		depthAttachment.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		render_pass.registerAttachments(
			colorAttachment, depthAttachment, g_buffer_position, g_buffer_normal, g_buffer_albedo );

		for ( int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			auto set { std::make_unique< DescriptorSet >( GBufferDescriptorSet::createLayout() ) };
			set->setMaxIDX( 2 );

			set->bindAttachment(
				0,
				*( g_buffer_position.resources().m_image_views[ i ].get() ),
				vk::ImageLayout::eShaderReadOnlyOptimal );

			set->bindAttachment(
				1, *( g_buffer_normal.resources().m_image_views[ i ].get() ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->bindAttachment(
				2, *( g_buffer_albedo.resources().m_image_views[ i ].get() ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->update();

			m_gbuffer_descriptor_set[ i ] = std::move( set );
		}

		static_assert( is_attachment< ColoredPresentAttachment > );
		static_assert( is_attachment< DepthAttachment > );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColoredPresentAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal > >
			g_buffer_subpass {
				0, depthAttachment, colorAttachment, g_buffer_position, g_buffer_normal, g_buffer_albedo
			};

		g_buffer_subpass.registerExternalDependency(
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests );

		g_buffer_subpass.registerExternalDependency(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColoredPresentAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > >
			present_subpass {
				1, depthAttachment, colorAttachment, g_buffer_position, g_buffer_normal, g_buffer_albedo
			};

		/*
		// This dependency transitions the input attachment from color attachment to input attachment read
		dependencies[ 2 ].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[ 2 ].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		dependencies[ 2 ].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		dependencies[ 2 ].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		dependencies[ 2 ].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		*/

		present_subpass.registerDependency(
			g_buffer_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

		present_subpass.registerDependency(
			present_subpass.getIndex(),
			VK_SUBPASS_EXTERNAL,
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eMemoryRead,
			vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::DependencyFlagBits::eByRegion );

		render_pass.registerSubpass( g_buffer_subpass );
		render_pass.registerSubpass( present_subpass );

		m_render_pass = render_pass.create();

		m_render_pass_resources = render_pass.resources( imageCount() );
		m_clear_values = render_pass.getClearValues();
	}

	void SwapChain::createFramebuffers()
	{
		m_swap_chain_buffers.resize( imageCount() );
		for ( uint8_t i = 0; i < imageCount(); i++ )
		{
			//TODO: Fix image shit. It's dying because the image is being nuked
			std::vector< vk::ImageView > attachments { m_render_pass_resources->forFrame( i ) };

			//Fill attachments for this frame
			const vk::Extent2D swapChainExtent { getSwapChainExtent() };
			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.renderPass = m_render_pass;
			framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if ( Device::getInstance()
			         .device()
			         .createFramebuffer( &framebufferInfo, nullptr, &( m_swap_chain_buffers[ i ] ) )
			     != vk::Result::eSuccess )
			{
				throw std::runtime_error( "failed to create framebuffer!" );
			}
		}
	}

	void SwapChain::createSyncObjects()
	{
		imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
		renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
		inFlightFences.resize( MAX_FRAMES_IN_FLIGHT );
		imagesInFlight.resize( imageCount(), VK_NULL_HANDLE );

		vk::SemaphoreCreateInfo semaphoreInfo {};

		vk::FenceCreateInfo fenceInfo {};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			auto vk_device { Device::getInstance().device() };
			if ( vk_device.createSemaphore( &semaphoreInfo, nullptr, &imageAvailableSemaphores[ i ] )
			     != vk::Result::eSuccess )
				throw std::runtime_error( "failed to create image available semaphore!" );
			if ( vk_device.createSemaphore( &semaphoreInfo, nullptr, &renderFinishedSemaphores[ i ] )
			     != vk::Result::eSuccess )
				throw std::runtime_error( "failed to create render finished semaphore!" );
			if ( vk_device.createFence( &fenceInfo, nullptr, &inFlightFences[ i ] ) != vk::Result::eSuccess )
				throw std::runtime_error( "failed to create in flight fence!" );
		}
	}

	vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >&
	                                                             availableFormats )
	{
		for ( const auto& availableFormat : availableFormats )
		{
			if ( availableFormat.format == vk::Format::eB8G8R8A8Srgb
			     && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
			{
				return availableFormat;
			}
		}

		return availableFormats[ 0 ];
	}

	vk::PresentModeKHR SwapChain::chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >&
	                                                         availablePresentModes )
	{
		for ( const auto& availablePresentMode : availablePresentModes )
		{
			if ( availablePresentMode == vk::PresentModeKHR::eMailbox )
			{
				std::cout << "Present mode: Mailbox" << std::endl;
				return availablePresentMode;
			}
		}

		// for (const auto &availablePresentMode : availablePresentModes) {
		//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		//     std::cout << "Present mode: Immediate" << std::endl;
		//     return availablePresentMode;
		//   }
		// }

		std::cout << "Present mode: V-Sync" << std::endl;
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D SwapChain::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities )
	{
		if ( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
		{
			return capabilities.currentExtent;
		}
		else
		{
			vk::Extent2D actualExtent = windowExtent;
			actualExtent.width = std::
				max( capabilities.minImageExtent.width,
			         std::min( capabilities.maxImageExtent.width, actualExtent.width ) );
			actualExtent.height = std::
				max( capabilities.minImageExtent.height,
			         std::min( capabilities.maxImageExtent.height, actualExtent.height ) );

			return actualExtent;
		}
	}

	vk::Format SwapChain::findDepthFormat()
	{
		return Device::getInstance().findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment );
	}

} // namespace fgl::engine
