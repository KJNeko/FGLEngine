#include "SwapChain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "Attachment.hpp"
#include "RenderPassBuilder.hpp"
#include "Subpass.hpp"
#include "engine/assets/TransferManager.hpp"

namespace fgl::engine
{

	SwapChain::SwapChain( const vk::Extent2D extent, PhysicalDevice& phy_device ) :
	  m_phy_device( phy_device ),
	  m_swapchain_support( Device::getInstance().getSwapChainSupport() ),
	  m_surface_format( chooseSwapSurfaceFormat( m_swapchain_support.formats ) ),
	  m_swap_chain_format( m_surface_format.format ),
	  m_swap_chain_depth_format( findDepthFormat() ),
	  m_swap_chain_extent( chooseSwapExtent( m_swapchain_support.capabilities ) ),
	  m_swapchain_extent( extent ),
	  m_swapchain( createSwapChain() ),
	  m_swap_chain_images( createSwapchainImages() ),
	  m_render_attachments( getSwapChainImageFormat(), findDepthFormat() ),
	  m_render_pass( createRenderPass() ),
	  m_swap_chain_buffers( createFramebuffers() ),
	  m_clear_values( populateAttachmentClearValues() ),
	  m_gbuffer_descriptor_set( createGBufferDescriptors() ),
	  m_gbuffer_composite_descriptor_set( createGBufferCompositeDescriptors() )
	{
		init();
	}

	SwapChain::SwapChain( const vk::Extent2D extent, std::shared_ptr< SwapChain > previous ) :
	  m_phy_device( previous->m_phy_device ),
	  old_swap_chain( previous ),
	  m_swapchain_support( Device::getInstance().getSwapChainSupport() ),
	  m_surface_format( chooseSwapSurfaceFormat( m_swapchain_support.formats ) ),
	  m_swap_chain_format( m_surface_format.format ),
	  m_swap_chain_depth_format( findDepthFormat() ),
	  m_swap_chain_extent( chooseSwapExtent( m_swapchain_support.capabilities ) ),
	  m_swapchain_extent( extent ),
	  m_swapchain( createSwapChain() ),
	  m_swap_chain_images( createSwapchainImages() ),
	  m_render_attachments( getSwapChainImageFormat(), findDepthFormat() ),
	  m_render_pass( createRenderPass() ),
	  m_swap_chain_buffers( createFramebuffers() ),
	  m_clear_values( populateAttachmentClearValues() ),
	  m_gbuffer_descriptor_set( createGBufferDescriptors() ),
	  m_gbuffer_composite_descriptor_set( createGBufferCompositeDescriptors() )
	{
		init();
		old_swap_chain.reset();
	}

	vk::raii::Framebuffer& SwapChain::getFrameBuffer( const FrameIndex frame_idx, const PresentIndex present_idx )
	{
		assert( present_idx < m_swap_chain_buffers.size() );
		assert( frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT );
		return m_swap_chain_buffers[ present_idx ][ frame_idx ];
	}

	void SwapChain::init()
	{
		createSyncObjects();
	}

	std::pair< vk::Result, std::uint32_t > SwapChain::acquireNextImage()
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { in_flight_fences[ current_frame_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		auto result { m_swapchain.acquireNextImage(
			std::numeric_limits< uint64_t >::max(),
			imageAvailableSemaphores[ current_frame_index ] // must be a not signaled semaphore
			) };

		return result;
	}

	vk::Result SwapChain::
		submitCommandBuffers( const vk::raii::CommandBuffer& buffers, const PresentIndex current_present_index )
	{
		ZoneScoped;

		images_in_flight[ current_present_index ] = in_flight_fences[ current_frame_index ];

		std::vector< vk::Fence > fences { images_in_flight[ current_present_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		vk::SubmitInfo submitInfo {};

		std::vector< vk::Semaphore > wait_sems { imageAvailableSemaphores[ current_frame_index ],
			                                     memory::TransferManager::getInstance().getFinishedSem() };

		std::vector< vk::PipelineStageFlags > wait_stages { vk::PipelineStageFlagBits::eColorAttachmentOutput,
			                                                vk::PipelineStageFlagBits::eTopOfPipe };

		submitInfo.setWaitSemaphores( wait_sems );
		submitInfo.setWaitDstStageMask( wait_stages );

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &( *buffers );

		std::vector< vk::Semaphore > signaled_semaphores { renderFinishedSemaphores[ current_frame_index ] };
		submitInfo.setSignalSemaphores( signaled_semaphores );

		Device::getInstance().device().resetFences( fences );

		std::vector< vk::SubmitInfo > submit_infos { submitInfo };

		Device::getInstance().graphicsQueue().submit( submitInfo, in_flight_fences[ current_frame_index ] );

		vk::PresentInfoKHR presentInfo = {};

		presentInfo.setWaitSemaphores( signaled_semaphores );

		std::vector< vk::SwapchainKHR > swapchains { m_swapchain };
		presentInfo.setSwapchains( swapchains );

		std::array< std::uint32_t, 1 > indicies { { current_present_index } };
		presentInfo.setImageIndices( indicies );

		if ( auto present_result = Device::getInstance().presentQueue().presentKHR( presentInfo );
		     present_result != vk::Result::eSuccess )
		{
			if ( present_result == vk::Result::eSuboptimalKHR ) return vk::Result::eSuboptimalKHR;

			throw std::runtime_error( "failed to present swap chain image!" );
		}

		current_frame_index = ( current_frame_index + 1 ) % MAX_FRAMES_IN_FLIGHT;

		return vk::Result::eSuccess;
	}

	vk::raii::SwapchainKHR SwapChain::createSwapChain()
	{
		ZoneScoped;

		const vk::PresentModeKHR present_mode { chooseSwapPresentMode( m_swapchain_support.presentModes ) };

		uint32_t image_count = m_swapchain_support.capabilities.minImageCount + 1;
		if ( m_swapchain_support.capabilities.maxImageCount > 0
		     && image_count > m_swapchain_support.capabilities.maxImageCount )
		{
			image_count = m_swapchain_support.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR create_info = {};
		create_info.surface = Device::getInstance().surface();

		create_info.minImageCount = image_count;
		create_info.imageFormat = m_surface_format.format;
		create_info.imageColorSpace = m_surface_format.colorSpace;
		create_info.imageExtent = m_swap_chain_extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		const std::uint32_t graphics_family { m_phy_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics ) };
		const std::uint32_t present_family { m_phy_device.queueInfo().getPresentIndex() };

		const uint32_t queueFamilyIndices[] = { graphics_family, present_family };

		if ( graphics_family != present_family )
		{
			// If the familys are not the same then the swapchain must be shared between
			// both queues.
			create_info.imageSharingMode = vk::SharingMode::eConcurrent;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			create_info.imageSharingMode = vk::SharingMode::eExclusive;
			create_info.queueFamilyIndexCount = 0; // Optional
			create_info.pQueueFamilyIndices = nullptr; // Optional
		}

		create_info.preTransform = m_swapchain_support.capabilities.currentTransform;
		create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		create_info.oldSwapchain = ( old_swap_chain == nullptr ? VK_NULL_HANDLE : *old_swap_chain->m_swapchain );

		return Device::getInstance()->createSwapchainKHR( create_info );
	}

	std::vector< std::shared_ptr< Image > > SwapChain::createSwapchainImages()
	{
		std::vector< vk::Image > swap_chain_images { m_swapchain.getImages() };
		std::vector< std::shared_ptr< Image > > images {};
		images.reserve( swap_chain_images.size() );

		for ( std::uint64_t i = 0; i < swap_chain_images.size(); ++i )
		{
			images.emplace_back( std::make_shared< Image >(
				m_swap_chain_extent,
				m_surface_format.format,
				swap_chain_images[ i ],
				vk::ImageUsageFlagBits::eColorAttachment ) );
		}

		assert( swap_chain_images.size() == images.size() );

		return images;
	}

	vk::raii::RenderPass SwapChain::createRenderPass()
	{
		ZoneScoped;

		RenderPassBuilder render_pass_builder {};

		render_pass_builder.registerAttachments(
			m_render_attachments.color,
			m_render_attachments.depth,
			m_gbuffer_attachments.position,
			m_gbuffer_attachments.normal,
			m_gbuffer_attachments.albedo,
			m_gbuffer_attachments.composite );

		static_assert( is_attachment< ColoredPresentAttachment > );
		static_assert( is_attachment< DepthAttachment > );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal > >
			g_buffer_subpass { 0,
			                   m_render_attachments.depth,
			                   m_gbuffer_attachments.position,
			                   m_gbuffer_attachments.normal,
			                   m_gbuffer_attachments.albedo };

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
			composite_subpass { 1,
			                    m_render_attachments.depth,
			                    m_gbuffer_attachments.composite,
			                    m_gbuffer_attachments.position,
			                    m_gbuffer_attachments.normal,
			                    m_gbuffer_attachments.albedo };

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

		// To prevent the composite buffer from getting obliterated by the gui pass and so we can use it to render to the GUI in certian areas, We need to keep them seperate and the composite image to be unmodified.
		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColoredPresentAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > >
			gui_subpass { 2, m_render_attachments.depth, m_render_attachments.color, m_gbuffer_attachments.composite };

		gui_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		/*
		g_buffer_subpass -> composite_subpass -> gui_subpass
		*/

		gui_subpass.registerDependencyFrom(
			composite_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

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

		render_pass_builder.registerSubpass( g_buffer_subpass );
		render_pass_builder.registerSubpass( composite_subpass );
		render_pass_builder.registerSubpass( gui_subpass );

		return render_pass_builder.create();
	}

	template < is_attachment... Attachments >
	std::vector< std::shared_ptr< ImageView > >
		fillViewsFromAttachments( uint8_t frame_idx, Attachments&&... attachments )
	{
		std::vector< std::shared_ptr< ImageView > > views {};

		views.resize( sizeof...( Attachments ) );

		( ( attachments.fillVec( frame_idx, views ) ), ... );

		return views;
	}

	std::vector< std::vector< vk::raii::Framebuffer > > SwapChain::createFramebuffers()
	{
		ZoneScoped;

		assert( imageCount() > 0 );

		m_render_attachments.color.linkImages( m_swap_chain_images );

		m_render_attachments.depth.createResources( imageCount(), getSwapChainExtent() );
		m_render_attachments.depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		// GBuffer resource management
		m_gbuffer_attachments.position.createResourceSpread(
			SwapChain::MAX_FRAMES_IN_FLIGHT, getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		m_gbuffer_attachments.normal.createResourceSpread(
			SwapChain::MAX_FRAMES_IN_FLIGHT, getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		m_gbuffer_attachments.albedo.createResourceSpread(
			SwapChain::MAX_FRAMES_IN_FLIGHT, getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );
		m_gbuffer_attachments.composite.createResourceSpread(
			SwapChain::MAX_FRAMES_IN_FLIGHT, getSwapChainExtent(), vk::ImageUsageFlagBits::eSampled );

		m_gbuffer_attachments.position.setClear( vk::ClearColorValue( std::array< float, 4 > {
			{ 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		m_gbuffer_attachments.normal.setClear( vk::ClearColorValue( std::array< float, 4 > {
			{ 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		m_gbuffer_attachments.albedo.setClear( vk::ClearColorValue( std::array< float, 4 > {
			{ 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		m_gbuffer_attachments.composite.setClear( vk::ClearColorValue( std::array< float, 4 > {
			{ 0.0f, 0.0f, 0.0f, 0.0f } } ) );
		;

		std::vector< std::vector< vk::raii::Framebuffer > > framebuffers {};

		for ( std::uint16_t present_idx = 0; present_idx < imageCount(); ++present_idx )
		{
			std::vector< vk::raii::Framebuffer > temp {};

			for ( FrameIndex frame_idx = 0; frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT; frame_idx++ )
			{
				std::vector< vk::ImageView > attachments {};

				attachments.emplace_back( *m_render_attachments.color.view( present_idx ) );
				attachments.emplace_back( *m_render_attachments.depth.view( present_idx ) );
				attachments.emplace_back( *m_gbuffer_attachments.position.view( frame_idx ) );
				attachments.emplace_back( *m_gbuffer_attachments.normal.view( frame_idx ) );
				attachments.emplace_back( *m_gbuffer_attachments.albedo.view( frame_idx ) );
				attachments.emplace_back( *m_gbuffer_attachments.composite.view( frame_idx ) );

				//Fill attachments for this frame
				const vk::Extent2D swapchain_extent { getSwapChainExtent() };
				vk::FramebufferCreateInfo framebufferInfo {};
				framebufferInfo.renderPass = m_render_pass;
				framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = swapchain_extent.width;
				framebufferInfo.height = swapchain_extent.height;
				framebufferInfo.layers = 1;

				temp.emplace_back( Device::getInstance()->createFramebuffer( framebufferInfo ) );
			}

			framebuffers.emplace_back( std::move( temp ) );
		}

		return framebuffers;
	}

	void SwapChain::createSyncObjects()
	{
		ZoneScoped;
		imageAvailableSemaphores.reserve( MAX_FRAMES_IN_FLIGHT );
		renderFinishedSemaphores.reserve( MAX_FRAMES_IN_FLIGHT );
		in_flight_fences.reserve( MAX_FRAMES_IN_FLIGHT );
		images_in_flight.resize( imageCount(), VK_NULL_HANDLE );

		vk::SemaphoreCreateInfo semaphoreInfo {};

		vk::FenceCreateInfo fenceInfo {};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			auto& device { Device::getInstance() };

			imageAvailableSemaphores.push_back( device->createSemaphore( semaphoreInfo ) );
			renderFinishedSemaphores.push_back( device->createSemaphore( semaphoreInfo ) );
			in_flight_fences.push_back( device->createFence( fenceInfo ) );
		}
	}

	vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >&
	                                                             available_formats )
	{
		ZoneScoped;
		for ( const auto& format : available_formats )
		{
			if ( format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
			{
				return format;
			}
		}

		return available_formats[ 0 ];
	}

	vk::PresentModeKHR SwapChain::chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >&
	                                                         available_present_modes )
	{
		ZoneScoped;
		for ( const auto& availablePresentMode : available_present_modes )
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

		for ( const auto& present_mode_khr : available_present_modes )
		{
			if ( present_mode_khr == vk::PresentModeKHR::eMailbox )
			{
				std::cout << "Present mode: Mailbox: ACTIVE" << std::endl;
				return present_mode_khr;
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

	std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT > SwapChain::
		createGBufferDescriptors()
	{
		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT > descriptors {};

		for ( FrameIndex frame_idx = 0; frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frame_idx )
		{
			auto gbuffer_set { std::make_unique< descriptors::DescriptorSet >( GBufferDescriptorSet::createLayout() ) };

			gbuffer_set->setMaxIDX( 2 );

			gbuffer_set->bindAttachment(
				0, m_gbuffer_attachments.position.view( frame_idx ), vk::ImageLayout::eShaderReadOnlyOptimal );

			gbuffer_set->bindAttachment(
				1, m_gbuffer_attachments.normal.view( frame_idx ), vk::ImageLayout::eShaderReadOnlyOptimal );

			gbuffer_set->bindAttachment(
				2, m_gbuffer_attachments.albedo.view( frame_idx ), vk::ImageLayout::eShaderReadOnlyOptimal );

			gbuffer_set->update();

			descriptors.at( frame_idx ) = std::move( gbuffer_set );
		}

		return descriptors;
	}

	std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT > SwapChain::
		createGBufferCompositeDescriptors()
	{
		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT > descriptors {};

		for ( FrameIndex frame_idx = 0; frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frame_idx )
		{
			auto composite_set {
				std::make_unique< descriptors::DescriptorSet >( GBufferCompositeDescriptorSet::createLayout() )
			};

			composite_set->setMaxIDX( 0 );
			composite_set->bindAttachment(
				0, m_gbuffer_attachments.composite.view( frame_idx ), vk::ImageLayout::eShaderReadOnlyOptimal );

			composite_set->update();

			m_gbuffer_composite_descriptor_set.at( frame_idx ) = std::move( composite_set );
		}

		return descriptors;
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
