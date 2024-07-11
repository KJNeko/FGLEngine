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
#include "engine/assets/TransferManager.hpp"

namespace fgl::engine
{

	SwapChain::SwapChain( vk::Extent2D extent, PhysicalDevice& phy_device ) :
	  m_phy_device( phy_device ),
	  m_swapchain_details( Device::getInstance().getSwapChainSupport() ),
	  m_surface_format( chooseSwapSurfaceFormat( m_swapchain_details.formats ) ),
	  m_present_mode( chooseSwapPresentMode( m_swapchain_details.presentModes ) ),
	  m_swapchain_extent( extent ),
	  m_swap_chain_format( m_surface_format.format ),
	  m_swap_chain_depth_format( findDepthFormat() ),
	  old_swap_chain( nullptr ),
	  m_swapchain( createSwapChain() ),
	  m_swap_chain_images( createSwapchainImages() ),
	  render_attachments( getSwapChainImageFormat(), findDepthFormat() )
	{
		init();
	}

	SwapChain::SwapChain( vk::Extent2D extent, std::shared_ptr< SwapChain > previous ) :
	  m_phy_device( previous->m_phy_device ),
	  m_swapchain_details( Device::getInstance().getSwapChainSupport() ),
	  m_surface_format( chooseSwapSurfaceFormat( m_swapchain_details.formats ) ),
	  m_present_mode( chooseSwapPresentMode( m_swapchain_details.presentModes ) ),
	  m_swapchain_extent( extent ),
	  m_swap_chain_format( m_surface_format.format ),
	  m_swap_chain_depth_format( findDepthFormat() ),
	  old_swap_chain( previous ),
	  m_swapchain( createSwapChain() ),
	  m_swap_chain_images( createSwapchainImages() ),
	  render_attachments( getSwapChainImageFormat(), findDepthFormat() )
	{
		init();
		old_swap_chain.reset();
	}

	void SwapChain::init()
	{
		createRenderPass();
		createFramebuffers();
		createSyncObjects();
	}

	std::pair< vk::Result, std::uint32_t > SwapChain::acquireNextImage()
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { in_flight_fence[ m_current_frame_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		auto result { m_swapchain.acquireNextImage(
			std::numeric_limits< uint64_t >::max(),
			imageAvailableSemaphores[ m_current_frame_index ] // must be a not signaled semaphore
			) };

		return result;
	}

	vk::Result SwapChain::submitCommandBuffers( const vk::raii::CommandBuffer& buffers, std::uint32_t imageIndex )
	{
		ZoneScoped;

		images_in_flight[ imageIndex ] = in_flight_fence[ m_current_frame_index ];

		std::vector< vk::Fence > fences { images_in_flight[ imageIndex ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		vk::SubmitInfo m_submit_info {};

		std::vector< vk::Semaphore > wait_sems { imageAvailableSemaphores[ m_current_frame_index ],
			                                     memory::TransferManager::getInstance().getFinishedSem() };

		std::vector< vk::PipelineStageFlags > wait_stages { vk::PipelineStageFlagBits::eColorAttachmentOutput,
			                                                vk::PipelineStageFlagBits::eTopOfPipe };

		m_submit_info.setWaitSemaphores( wait_sems );
		m_submit_info.setWaitDstStageMask( wait_stages );

		m_submit_info.commandBufferCount = 1;
		m_submit_info.pCommandBuffers = &( *buffers );

		std::vector< vk::Semaphore > signaled_semaphores { renderFinishedSemaphores[ m_current_frame_index ] };
		m_submit_info.setSignalSemaphores( signaled_semaphores );

		Device::getInstance().device().resetFences( fences );

		std::vector< vk::SubmitInfo > submit_infos { m_submit_info };

		Device::getInstance().graphicsQueue().submit( m_submit_info, in_flight_fence[ m_current_frame_index ] );

		vk::PresentInfoKHR presentInfo = {};

		presentInfo.setWaitSemaphores( signaled_semaphores );

		std::vector< vk::SwapchainKHR > swapchains { m_swapchain };
		presentInfo.setSwapchains( swapchains );

		std::array< std::uint32_t, 1 > indicies { { imageIndex } };
		presentInfo.setImageIndices( indicies );

		if ( auto present_result = Device::getInstance().presentQueue().presentKHR( presentInfo );
		     present_result != vk::Result::eSuccess )
		{
			if ( present_result == vk::Result::eSuboptimalKHR ) return vk::Result::eSuboptimalKHR;

			throw std::runtime_error( "failed to present swap chain image!" );
		}

		m_current_frame_index = ( m_current_frame_index + 1 ) % MAX_FRAMES_IN_FLIGHT;

		return vk::Result::eSuccess;
	}

	vk::raii::SwapchainKHR SwapChain::createSwapChain()
	{
		ZoneScoped;

		std::uint32_t image_count { m_swapchain_details.capabilities.minImageCount + 1 };
		if ( m_swapchain_details.capabilities.maxImageCount > 0
		     && image_count > m_swapchain_details.capabilities.maxImageCount )
		{
			image_count = m_swapchain_details.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo = {};
		createInfo.surface = Device::getInstance().surface();

		createInfo.minImageCount = image_count;
		createInfo.imageFormat = m_surface_format.format;
		createInfo.imageColorSpace = m_surface_format.colorSpace;
		createInfo.imageExtent = m_swapchain_extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		const std::uint32_t graphics_family { m_phy_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics ) };
		const std::uint32_t present_family { m_phy_device.queueInfo().getPresentIndex() };

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

		createInfo.preTransform = m_swapchain_details.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

		createInfo.presentMode = m_present_mode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = old_swap_chain == nullptr ? VK_NULL_HANDLE : *old_swap_chain->m_swapchain;

		return Device::getInstance()->createSwapchainKHR( createInfo );
	}

	std::vector< Image > SwapChain::createSwapchainImages()
	{
		std::vector< vk::Image > swap_chain_images { m_swapchain.getImages() };
		std::vector< Image > images {};

		for ( std::uint64_t i = 0; i < swap_chain_images.size(); i++ )
		{
			auto& itter = images.emplace_back(
				m_swapchain_extent,
				m_surface_format.format,
				swap_chain_images[ i ],
				vk::ImageUsageFlagBits::eColorAttachment );
			itter.setName( "Swapchain image: " + std::to_string( i ) );
		}

		return images;
	}

	void SwapChain::createRenderPass()
	{
		ZoneScoped;
		//Present attachment

		for ( int i = 0; i < imageCount(); ++i )
		{
			auto& image { m_swap_chain_images[ i ] };

			image.setName( "SwapChainImage: " + std::to_string( i ) );
		}

		render_attachments.color.linkImages( m_swap_chain_images );

		gbuffer.position.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		gbuffer.normal.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		gbuffer.albedo.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		gbuffer.composite.createResourceSpread( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );

		gbuffer.position.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		gbuffer.normal.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		gbuffer.albedo.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		gbuffer.composite.setClear( vk::ClearColorValue( std::array< float, 4 > { { 0.0f, 0.0f, 0.0f, 0.0f } } ) );

		g_buffer_position_img = std::make_unique< Texture >( gbuffer.position.m_attachment_resources.m_images[ 0 ]
		                                                         ->setName( "GBufferPosition" ) );
		g_buffer_normal_img = std::make_unique< Texture >( gbuffer.normal.m_attachment_resources.m_images[ 0 ]
		                                                       ->setName( "GBufferNormal" ) );
		g_buffer_albedo_img = std::make_unique< Texture >( gbuffer.albedo.m_attachment_resources.m_images[ 0 ]
		                                                       ->setName( "GBufferAlbedo" ) );
		g_buffer_composite_img = std::make_unique< Texture >( gbuffer.composite.m_attachment_resources.m_images[ 0 ]
		                                                          ->setName( "GBufferComposite" ) );

		RenderPass render_pass {};

		render_attachments.depth.createResources( imageCount(), getSwapChainExtent() );
		render_attachments.depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		render_pass.registerAttachments(
			render_attachments.color,
			render_attachments.depth,
			gbuffer.position,
			gbuffer.normal,
			gbuffer.albedo,
			gbuffer.composite );

		for ( int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			{
				auto set { std::make_unique< descriptors::DescriptorSet >( GBufferDescriptorSet::createLayout() ) };

				set->setMaxIDX( 2 );

				set->bindAttachment(
					0,
					*( gbuffer.position.resources().m_image_views[ i ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				set->bindAttachment(
					1,
					*( gbuffer.normal.resources().m_image_views[ i ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				set->bindAttachment(
					2,
					*( gbuffer.albedo.resources().m_image_views[ i ].get() ),
					vk::ImageLayout::eShaderReadOnlyOptimal );

				set->update();

				m_gbuffer_descriptor_set[ i ] = std::move( set );
			}

			{
				auto composite_set {
					std::make_unique< descriptors::DescriptorSet >( GBufferCompositeDescriptorSet::createLayout() )
				};

				composite_set->setMaxIDX( 1 );
				composite_set->bindAttachment(
					0,
					*( gbuffer.composite.resources().m_image_views[ i ].get() ),
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
			g_buffer_subpass { 0, render_attachments.depth, gbuffer.position, gbuffer.normal, gbuffer.albedo };

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
				1, render_attachments.depth, gbuffer.composite, gbuffer.position, gbuffer.normal, gbuffer.albedo
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
			gui_subpass { 2, render_attachments.depth, render_attachments.color, gbuffer.composite };

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
		in_flight_fence.reserve( MAX_FRAMES_IN_FLIGHT );
		images_in_flight.resize( imageCount(), VK_NULL_HANDLE );

		vk::SemaphoreCreateInfo semaphoreInfo {};

		vk::FenceCreateInfo fenceInfo {};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			auto& device { Device::getInstance() };

			imageAvailableSemaphores.push_back( device->createSemaphore( semaphoreInfo ) );
			renderFinishedSemaphores.push_back( device->createSemaphore( semaphoreInfo ) );
			in_flight_fence.push_back( device->createFence( fenceInfo ) );
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
			vk::Extent2D actualExtent = m_swapchain_extent;
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
