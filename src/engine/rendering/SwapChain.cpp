#include "SwapChain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "pipelines/Attachment.hpp"
#include "renderpass/RenderPass.hpp"

namespace fgl::engine
{
	SwapChain::SwapChain( const vk::Extent2D extent, PhysicalDevice& phy_device ) :
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
	  render_attachments( getSwapChainImageFormat(), findDepthFormat() ),
	  m_render_pass( createRenderPass() ),
	  m_swap_chain_buffers( createFramebuffers() ),
	  // m_input_descriptors( createInputDescriptors() ),
	  m_clear_values(
		  // gatherClearValues( render_attachments.color, render_attachments.depth, render_attachments.input_color ) )
		  gatherClearValues( render_attachments.color, render_attachments.depth ) )
	{
		init();
	}

	SwapChain::SwapChain( const vk::Extent2D extent, std::shared_ptr< SwapChain > previous ) :
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
	  render_attachments( getSwapChainImageFormat(), findDepthFormat() ),
	  m_render_pass( createRenderPass() ),
	  m_swap_chain_buffers( createFramebuffers() ),
	  // m_input_descriptors( createInputDescriptors() ),
	  m_clear_values(
		  // gatherClearValues( render_attachments.color, render_attachments.depth, render_attachments.input_color ) )
		  gatherClearValues( render_attachments.color, render_attachments.depth ) )
	{
		init();
		old_swap_chain.reset();
	}

	std::vector< std::unique_ptr< descriptors::DescriptorSet > > SwapChain::createInputDescriptors()
	{
		std::vector< std::unique_ptr< descriptors::DescriptorSet > > data {};
		data.resize( imageCount() );

		for ( PresentIndex i = 0; i < imageCount(); ++i )
		{
			auto set { gui_descriptor_set.create() };
			//auto set { std::make_unique< descriptors::DescriptorSet >( GuiInputDescriptorSet::createLayout() ) };

			// set->bindAttachment(
			// 0, render_attachments.input_color.getView( i ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->update();

			data[ i ] = std::move( set );
		}

		return data;
	}

	void SwapChain::init()
	{
		createSyncObjects();

		// render_attachments.input_color.setName( "Input Color" );
	}

	std::pair< vk::Result, PresentIndex > SwapChain::acquireNextImage()
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { in_flight_fence[ m_current_frame_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		auto result { m_swapchain.acquireNextImage(
			std::numeric_limits< uint64_t >::max(),
			image_available_sem[ m_current_frame_index ] // must be a not signaled semaphore
			) };

		return result;
	}

	vk::Result SwapChain::
		submitCommandBuffers( const vk::raii::CommandBuffer& buffers, const PresentIndex present_index )
	{
		ZoneScoped;

		images_in_flight[ present_index ] = in_flight_fence[ m_current_frame_index ];

		std::vector< vk::Fence > fences { images_in_flight[ present_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		vk::SubmitInfo m_submit_info {};

		std::vector< vk::Semaphore > wait_sems { image_available_sem[ m_current_frame_index ],
			                                     memory::TransferManager::getInstance().getFinishedSem() };

		std::vector< vk::PipelineStageFlags > wait_stages { vk::PipelineStageFlagBits::eColorAttachmentOutput,
			                                                vk::PipelineStageFlagBits::eTopOfPipe };

		m_submit_info.setWaitSemaphores( wait_sems );
		m_submit_info.setWaitDstStageMask( wait_stages );

		m_submit_info.commandBufferCount = 1;
		m_submit_info.pCommandBuffers = &( *buffers );

		std::vector< vk::Semaphore > signaled_semaphores { render_finished_sem[ m_current_frame_index ] };
		m_submit_info.setSignalSemaphores( signaled_semaphores );

		Device::getInstance().device().resetFences( fences );

		std::vector< vk::SubmitInfo > submit_infos { m_submit_info };

		Device::getInstance().graphicsQueue().submit( m_submit_info, in_flight_fence[ m_current_frame_index ] );

		vk::PresentInfoKHR presentInfo = {};

		presentInfo.setWaitSemaphores( signaled_semaphores );

		std::vector< vk::SwapchainKHR > swapchains { m_swapchain };
		presentInfo.setSwapchains( swapchains );

		std::array< std::uint32_t, 1 > indicies { { present_index } };
		presentInfo.setImageIndices( indicies );

		if ( auto present_result = Device::getInstance().presentQueue().presentKHR( presentInfo );
		     present_result != vk::Result::eSuccess )
		{
			if ( present_result == vk::Result::eSuboptimalKHR ) return vk::Result::eSuboptimalKHR;

			throw std::runtime_error( "failed to present swap chain image!" );
		}

		m_current_frame_index = static_cast<
			FrameIndex >( ( m_current_frame_index + static_cast< FrameIndex >( 1 ) ) % MAX_FRAMES_IN_FLIGHT );

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

	vk::raii::RenderPass SwapChain::createRenderPass()
	{
		ZoneScoped;
		//Present attachment

		rendering::RenderPassBuilder builder;

		constexpr std::size_t ColorIndex { 0 };
		constexpr std::size_t DepthIndex { 1 };
		// constexpr std::size_t InputColorIndex { 2 };

		// builder.setAttachmentCount( 3 );
		builder.setAttachmentCount( 2 );

		auto color { builder.attachment( ColorIndex ) };

		color.setFormat( SwapChain::getSwapChainImageFormat() );
		color.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR );
		color.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		auto depth { builder.attachment( DepthIndex ) };

		depth.setFormat( SwapChain::findDepthFormat() );
		depth.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal );
		depth.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare );

		// auto color_input { builder.attachment( InputColorIndex ) };
		// color_input.setFormat( vk::Format::eR8G8B8A8Unorm );
		// color_input.setLayouts( vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal );
		// color_input.setOps( vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eDontCare );

		auto& gui_subpass { builder.createSubpass( 0 ) };

		// gui_subpass.addInputLayout( InputColorIndex, vk::ImageLayout::eShaderReadOnlyOptimal );
		gui_subpass.setDepthLayout( DepthIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal );
		gui_subpass.addRenderLayout( ColorIndex, vk::ImageLayout::eColorAttachmentOptimal );

		gui_subpass.addDependencyFromExternal(
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests );

		gui_subpass.addDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		return builder.create();
	}

	std::vector< vk::raii::Framebuffer > SwapChain::createFramebuffers()
	{
		ZoneScoped;

		render_attachments.color.linkImages( m_swap_chain_images );

		render_attachments.depth.createResources( imageCount(), getSwapChainExtent() );
		render_attachments.depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		// render_attachments.input_color
		// .createResources( imageCount(), getSwapChainExtent(), vk::ImageUsageFlagBits::eTransferDst );

		std::vector< vk::raii::Framebuffer > framebuffers {};

		framebuffers.reserve( imageCount() );

		for ( uint8_t i = 0; i < imageCount(); i++ )
		{
			std::vector< vk::ImageView > attachments { getViewsForFrame(
				// i, render_attachments.color, render_attachments.depth, render_attachments.input_color ) };
				i,
				render_attachments.color,
				render_attachments.depth ) };

			//Fill attachments for this frame
			const vk::Extent2D swapChainExtent { getSwapChainExtent() };
			vk::FramebufferCreateInfo framebufferInfo {};
			framebufferInfo.renderPass = m_render_pass;
			framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			framebuffers.push_back( Device::getInstance()->createFramebuffer( framebufferInfo ) );
		}

		return framebuffers;
	}

	void SwapChain::createSyncObjects()
	{
		ZoneScoped;
		image_available_sem.reserve( imageCount() );
		render_finished_sem.reserve( imageCount() );
		in_flight_fence.reserve( imageCount() );
		images_in_flight.resize( imageCount(), VK_NULL_HANDLE );

		vk::SemaphoreCreateInfo semaphoreInfo {};

		vk::FenceCreateInfo fenceInfo {};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			auto& device { Device::getInstance() };

			image_available_sem.push_back( device->createSemaphore( semaphoreInfo ) );
			render_finished_sem.push_back( device->createSemaphore( semaphoreInfo ) );
			in_flight_fence.push_back( device->createFence( fenceInfo ) );
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

	vk::PresentModeKHR SwapChain::chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& present_modes )
	{
		ZoneScoped;
		for ( const auto& mode : present_modes )
		{
			switch ( mode )
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

		for ( const auto& mode : present_modes )
		{
			if ( mode == vk::PresentModeKHR::eMailbox )
			{
				std::cout << "Present mode: Mailbox: ACTIVE" << std::endl;
				return mode;
			}
		}

		std::cout << "Present mode: V-Sync: ACTIVE" << std::endl;
		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D SwapChain::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const
	{
		ZoneScoped;
		if ( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
		{
			return capabilities.currentExtent;
		}
		else
		{
			vk::Extent2D actual_extent { m_swapchain_extent };

			const auto [ min_height, min_width ] = capabilities.minImageExtent;
			const auto [ max_height, max_width ] = capabilities.maxImageExtent;

			actual_extent.width = std::clamp( actual_extent.width, min_width, max_width );

			actual_extent.height = std::clamp( actual_extent.height, min_height, max_height );

			return actual_extent;
		}
	}

	SwapChain::~SwapChain()
	{}

	/*
	descriptors::DescriptorSet& SwapChain::getInputDescriptor( const PresentIndex present_index )
	{
		assert( present_index < m_input_descriptors.size() );
		return *m_input_descriptors[ present_index ];
	}
	*/

	// Image& SwapChain::getInputImage( const PresentIndex present_index ) const
	// {
	// return *render_attachments.input_color.m_attachment_resources.m_images[ present_index ];
	// }

	vk::raii::Framebuffer& SwapChain::getFrameBuffer( const PresentIndex present_index )
	{
		return m_swap_chain_buffers[ static_cast< std::size_t >( present_index ) ];
	}

	bool SwapChain::compareSwapFormats( const SwapChain& other ) const
	{
		return m_swap_chain_depth_format == other.m_swap_chain_depth_format
		    && m_swap_chain_format == other.m_swap_chain_format;
	}

	float SwapChain::extentAspectRatio() const
	{
		return static_cast< float >( m_swapchain_extent.width ) / static_cast< float >( m_swapchain_extent.height );
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
