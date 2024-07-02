#include "SwapChain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "Attachment.hpp"
#include "RenderPass.hpp"
#include "Subpass.hpp"

namespace fgl::engine
{

	SwapChain::SwapChain( vk::Extent2D extent, PhysicalDevice& phy_device ) :
	  m_phy_device( phy_device ),
	  windowExtent( extent )
	{
		init();
	}

	SwapChain::SwapChain( vk::Extent2D extent, std::shared_ptr< SwapChain > previous ) :
	  m_phy_device( previous->m_phy_device ),
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

	std::pair< vk::Result, std::uint32_t > SwapChain::acquireNextImage()
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { inFlightFences[ currentFrame ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		std::uint32_t image_idx { 0 };

		auto result { swapChain.acquireNextImage(
			std::numeric_limits< uint64_t >::max(),
			imageAvailableSemaphores[ currentFrame ] // must be a not signaled semaphore
			) };

		return result;
	}

	vk::Result SwapChain::submitCommandBuffers( const vk::raii::CommandBuffer& buffers, std::uint32_t imageIndex )
	{
		ZoneScoped;

		imagesInFlight[ imageIndex ] = inFlightFences[ currentFrame ];

		std::vector< vk::Fence > fences { imagesInFlight[ imageIndex ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		vk::SubmitInfo submitInfo {};

		std::vector< vk::Semaphore > wait_sems { imageAvailableSemaphores[ currentFrame ],
			                                     TransferManager::getInstance().getFinishedSem() };

		std::vector< vk::PipelineStageFlags > wait_stages { vk::PipelineStageFlagBits::eColorAttachmentOutput,
			                                                vk::PipelineStageFlagBits::eTopOfPipe };

		submitInfo.setWaitSemaphores( wait_sems );
		submitInfo.setWaitDstStageMask( wait_stages );

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &( *buffers );

		std::vector< vk::Semaphore > signaled_semaphores { renderFinishedSemaphores[ currentFrame ] };
		submitInfo.setSignalSemaphores( signaled_semaphores );

		Device::getInstance().device().resetFences( fences );

		std::vector< vk::SubmitInfo > submit_infos { submitInfo };

		Device::getInstance().graphicsQueue().submit( submitInfo, inFlightFences[ currentFrame ] );

		vk::PresentInfoKHR presentInfo = {};

		presentInfo.setWaitSemaphores( signaled_semaphores );

		std::vector< vk::SwapchainKHR > swapchains { swapChain };
		presentInfo.setSwapchains( swapchains );

		std::array< std::uint32_t, 1 > indicies { { imageIndex } };
		presentInfo.setImageIndices( indicies );

		if ( auto present_result = Device::getInstance().presentQueue().presentKHR( presentInfo );
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
		ZoneScoped;
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

		std::uint32_t graphics_family { m_phy_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics ) };
		std::uint32_t present_family { m_phy_device.queueInfo().getPresentIndex() };

		const uint32_t queueFamilyIndices[] = { graphics_family, present_family };

		if ( graphics_family != present_family )
		{
			// If the familys are not the same then the swapchain must be shared between
			// both queues.
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

		createInfo.oldSwapchain = old_swap_chain == nullptr ? VK_NULL_HANDLE : *old_swap_chain->swapChain;

		swapChain = Device::getInstance()->createSwapchainKHR( createInfo );

		std::vector< vk::Image > swap_chain_images { swapChain.getImages() };

		for ( std::uint64_t i = 0; i < swap_chain_images.size(); i++ )
		{
			auto& itter =
				m_swap_chain_images
					.emplace_back( extent, surfaceFormat.format, swap_chain_images[ i ], createInfo.imageUsage );
			itter.setName( "Swapchain image: " + std::to_string( i ) );
		}

		m_swap_chain_format = surfaceFormat.format;
		m_swap_chain_extent = extent;
	}

	void SwapChain::createRenderPass()
	{
		ZoneScoped;
		//Present attachment
		ColoredPresentAttachment colorAttachment { getSwapChainImageFormat() };

		for ( int i = 0; i < imageCount(); ++i )
		{
			auto& image { m_swap_chain_images[ i ] };

			image.setName( "SwapChainImage: " + std::to_string( i ) );
		}

		colorAttachment.linkImages( m_swap_chain_images );

		// G-Buffer
		ColorAttachment g_buffer_position { vk::Format::eR16G16B16A16Sfloat };
		ColorAttachment g_buffer_normal { vk::Format::eR16G16B16A16Sfloat };
		ColorAttachment g_buffer_albedo { vk::Format::eR8G8B8A8Unorm };
		ColorAttachment g_buffer_composite { vk::Format::eR8G8B8A8Unorm };

		g_buffer_position.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		g_buffer_normal.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		g_buffer_albedo.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		g_buffer_composite.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );

		g_buffer_position.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		g_buffer_normal.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		g_buffer_albedo.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		g_buffer_composite.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );

		g_buffer_position_img = std::make_unique< Texture >( g_buffer_position.m_attachment_resources.m_images[ 0 ]
		                                                         ->setName( "GBufferPosition" ) );
		g_buffer_normal_img = std::make_unique< Texture >( g_buffer_normal.m_attachment_resources.m_images[ 0 ]
		                                                       ->setName( "GBufferNormal" ) );
		g_buffer_albedo_img = std::make_unique< Texture >( g_buffer_albedo.m_attachment_resources.m_images[ 0 ]
		                                                       ->setName( "GBufferAlbedo" ) );
		g_buffer_composite_img = std::make_unique< Texture >( g_buffer_composite.m_attachment_resources.m_images[ 0 ]
		                                                          ->setName( "GBufferComposite" ) );

		RenderPass render_pass {};

		DepthAttachment depthAttachment { findDepthFormat() };
		depthAttachment.createResources( imageCount(), getSwapChainExtent() );
		depthAttachment.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		render_pass.registerAttachments(
			colorAttachment, depthAttachment, g_buffer_position, g_buffer_normal, g_buffer_albedo, g_buffer_composite );

		for ( int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			{
				auto set { std::make_unique< DescriptorSet >( GBufferDescriptorSet::createLayout() ) };

				set->setMaxIDX( 2 );

				set->bindAttachment(
					0,
					*( g_buffer_position.resources().m_image_views[ i ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				set->bindAttachment(
					1,
					*( g_buffer_normal.resources().m_image_views[ i ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				set->bindAttachment(
					2,
					*( g_buffer_albedo.resources().m_image_views[ i ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				set->update();

				m_gbuffer_descriptor_set[ i ] = std::move( set );
			}

			{
				auto composite_set {
					std::make_unique< DescriptorSet >( GBufferCompositeDescriptorSet::createLayout() )
				};

				composite_set->setMaxIDX( 2 );
				composite_set->bindAttachment(
					0,
					*( g_buffer_composite.resources().m_image_views[ 0 ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				composite_set->update();

				m_gbuffer_composite_descriptor_set[ i ] = std::move( composite_set );
			}
		}

		static_assert( is_attachment< ColoredPresentAttachment > );
		static_assert( is_attachment< DepthAttachment > );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal > >
			g_buffer_subpass { 0, depthAttachment, g_buffer_position, g_buffer_normal, g_buffer_albedo };

		g_buffer_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests );

		g_buffer_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > >
			composite_subpass {
				1, depthAttachment, g_buffer_composite, g_buffer_position, g_buffer_normal, g_buffer_albedo
			};

		composite_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		// For color attachments
		composite_subpass.registerDependencyFrom(
			g_buffer_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

		// For depth attachment
		composite_subpass.registerDependencyFrom(
			g_buffer_subpass,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::DependencyFlagBits::eByRegion );

		/*
		composite_subpass.registerDependencyFrom(
			g_buffer_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eTransferWrite,
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::DependencyFlagBits::eByRegion );
		*/

		// To prevent the composite buffer from getting obliterated by the gui pass and so we can use it to render to the GUI in certian areas, We need to keep them seperate and the composite image to be unmodified.
		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColoredPresentAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > >
			gui_subpass { 2, depthAttachment, colorAttachment, g_buffer_composite };

		gui_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		/*
		g_buffer_subpass -> composite_subpass -> gui_subpass
		*/

		gui_subpass.registerDependencyFrom(
			composite_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eShaderRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

		//composite_subpass.registerFullDependency( g_buffer_subpass );
		//gui_subpass.registerFullDependency( composite_subpass );

		gui_subpass.registerDependencyFrom(
			composite_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::DependencyFlagBits::eByRegion );

		gui_subpass.registerDependencyToExternal(
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eMemoryRead,
			vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::DependencyFlagBits::eByRegion );

		render_pass.registerSubpass( g_buffer_subpass );
		render_pass.registerSubpass( composite_subpass );
		render_pass.registerSubpass( gui_subpass );

		m_render_pass = render_pass.create();

		m_render_pass_resources = render_pass.resources( imageCount() );
		m_clear_values = render_pass.getClearValues();
	}

	void SwapChain::createFramebuffers()
	{
		ZoneScoped;
		m_swap_chain_buffers.clear();
		m_swap_chain_buffers.reserve( imageCount() );
		for ( uint8_t i = 0; i < imageCount(); i++ )
		{
			std::vector< vk::ImageView > attachments { m_render_pass_resources->forFrame( i ) };

			//Fill attachments for this frame
			const vk::Extent2D swapChainExtent { getSwapChainExtent() };
			vk::FramebufferCreateInfo framebufferInfo {};
			framebufferInfo.renderPass = m_render_pass;
			framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			m_swap_chain_buffers.push_back( Device::getInstance()->createFramebuffer( framebufferInfo ) );
		}
	}

	void SwapChain::createSyncObjects()
	{
		ZoneScoped;
		imageAvailableSemaphores.reserve( MAX_FRAMES_IN_FLIGHT );
		renderFinishedSemaphores.reserve( MAX_FRAMES_IN_FLIGHT );
		inFlightFences.reserve( MAX_FRAMES_IN_FLIGHT );
		imagesInFlight.resize( imageCount(), VK_NULL_HANDLE );

		vk::SemaphoreCreateInfo semaphoreInfo {};

		vk::FenceCreateInfo fenceInfo {};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			auto& device { Device::getInstance() };

			imageAvailableSemaphores.push_back( device->createSemaphore( semaphoreInfo ) );
			renderFinishedSemaphores.push_back( device->createSemaphore( semaphoreInfo ) );
			inFlightFences.push_back( device->createFence( fenceInfo ) );
		}
	}

	vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >&
	                                                             availableFormats )
	{
		ZoneScoped;
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
		ZoneScoped;
		for ( const auto& availablePresentMode : availablePresentModes )
		{
			switch ( availablePresentMode )
			{
				case vk::PresentModeKHR::eImmediate:
					std::cout << "Present mode: Immediate" << std::endl;
					break;
				case vk::PresentModeKHR::eMailbox:
					std::cout << "Present mode: Mailbox" << std::endl;
					break;
				case vk::PresentModeKHR::eFifo:
					std::cout << "Present mode: V-Sync" << std::endl;
					break;
				case vk::PresentModeKHR::eFifoRelaxed:
					std::cout << "Present mode: V-Sync: RELAXED" << std::endl;
					break;
				case vk::PresentModeKHR::eSharedDemandRefresh:
					std::cout << "Present mode: Shared Demand Refresh" << std::endl;
					break;
				case vk::PresentModeKHR::eSharedContinuousRefresh:
					std::cout << "Present mode: Shared Continuous Refresh" << std::endl;
					break;
			}
		}

		for ( const auto& availablePresentMode : availablePresentModes )
		{
			if ( availablePresentMode == vk::PresentModeKHR::eMailbox )
			{
				std::cout << "Present mode: Mailbox: ACTIVE" << std::endl;
				return availablePresentMode;
			}
		}

		// for (const auto &availablePresentMode : availablePresentModes) {
		//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		//     std::cout << "Present mode: Immediate" << std::endl;
		//     return availablePresentMode;
		//   }
		// }

		std::cout << "Present mode: V-Sync: ACTIVE" << std::endl;
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D SwapChain::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities )
	{
		ZoneScoped;
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
		ZoneScoped;
		return Device::getInstance().findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment );
	}

} // namespace fgl::engine
