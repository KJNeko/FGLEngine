#include "PresentSwapChain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "RenderingFormats.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "pipelines/Attachment.hpp"
#include "renderpass/RenderPass.hpp"

namespace fgl::engine
{
	PresentSwapChain::PresentSwapChain( const vk::Extent2D extent, PhysicalDevice& phy_device ) :
	  m_phy_device( phy_device ),
	  m_swapchain_details( Device::getInstance().getSwapChainSupport() ),
	  m_surface_format( chooseSwapSurfaceFormat( m_swapchain_details.formats ) ),
	  m_present_mode( chooseSwapPresentMode( m_swapchain_details.presentModes ) ),
	  m_swapchain_extent( extent ),
	  m_swap_chain_format( m_surface_format.format ),
	  m_swap_chain_depth_format( pickDepthFormat() ),
	  m_old_swap_chain( nullptr ),
	  m_swapchain( createSwapChain() ),
	  m_swap_chain_images( createSwapchainImages() ),
	  m_render_attachments( m_swap_chain_format, pickDepthFormat() ),
	  // m_input_descriptors( createInputDescriptors() ),
	  m_clear_values(
		  // gatherClearValues( render_attachments.color, render_attachments.depth, render_attachments.input_color ) )
		  gatherClearValues( m_render_attachments.m_color, m_render_attachments.m_depth ) )
	{
		init();

		m_render_attachments.m_color.linkImages( m_swap_chain_images );
		m_render_attachments.m_color.setName( "PresetnSwapChain::color" );

		m_render_attachments.m_depth.createResources( imageCount(), getSwapChainExtent() );
		m_render_attachments.m_depth.setName( "PresentSwapChain::Depth" );
		m_render_attachments.m_depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );
	}

	PresentSwapChain::PresentSwapChain( const vk::Extent2D extent, std::shared_ptr< PresentSwapChain > previous ) :
	  m_phy_device( previous->m_phy_device ),
	  m_swapchain_details( Device::getInstance().getSwapChainSupport() ),
	  m_surface_format( chooseSwapSurfaceFormat( m_swapchain_details.formats ) ),
	  m_present_mode( chooseSwapPresentMode( m_swapchain_details.presentModes ) ),
	  m_swapchain_extent( extent ),
	  m_swap_chain_format( m_surface_format.format ),
	  m_swap_chain_depth_format( pickDepthFormat() ),
	  m_old_swap_chain( previous ),
	  m_swapchain( createSwapChain() ),
	  m_swap_chain_images( createSwapchainImages() ),
	  m_render_attachments( m_swap_chain_format, m_swap_chain_depth_format ),
	  // m_input_descriptors( createInputDescriptors() ),
	  m_clear_values(
		  // gatherClearValues( render_attachments.color, render_attachments.depth, render_attachments.input_color ) )
		  gatherClearValues( m_render_attachments.m_color, m_render_attachments.m_depth ) )
	{
		init();

		m_render_attachments.m_color.linkImages( m_swap_chain_images );
		m_render_attachments.m_color.setName( "PresetnSwapChain::color" );

		m_render_attachments.m_depth.createResources( imageCount(), getSwapChainExtent() );
		m_render_attachments.m_depth.setName( "PresentSwapChain::Depth" );
		m_render_attachments.m_depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		m_old_swap_chain.reset();
	}

	std::vector< std::unique_ptr< descriptors::DescriptorSet > > PresentSwapChain::createInputDescriptors()
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

	void PresentSwapChain::init()
	{
		createSyncObjects();

		// render_attachments.input_color.setName( "Input Color" );
	}

	std::pair< vk::Result, PresentIndex > PresentSwapChain::acquireNextImage()
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { m_in_flight_fence[ m_current_frame_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		auto result { m_swapchain.acquireNextImage(
			std::numeric_limits< uint64_t >::max(),
			m_image_available_sem[ m_current_frame_index ] // must be a not signaled semaphore
			) };

		return result;
	}

	vk::Result PresentSwapChain::submitCommandBuffers( const CommandBuffer& buffers, const PresentIndex present_index )
	{
		ZoneScoped;

		m_images_in_flight[ present_index ] = m_in_flight_fence[ m_current_frame_index ];

		std::vector< vk::Fence > fences { m_images_in_flight[ present_index ] };

		if ( Device::getInstance().device().waitForFences( fences, VK_TRUE, std::numeric_limits< uint64_t >::max() )
		     != vk::Result::eSuccess )
			throw std::runtime_error( "failed to wait for fences!" );

		vk::SubmitInfo m_submit_info {};

		std::vector< vk::Semaphore > wait_sems { m_image_available_sem[ m_current_frame_index ],
			                                     memory::TransferManager::getInstance().getFinishedSem() };

		std::vector< vk::PipelineStageFlags > wait_stages { vk::PipelineStageFlagBits::eColorAttachmentOutput,
			                                                vk::PipelineStageFlagBits::eTopOfPipe };

		m_submit_info.setWaitSemaphores( wait_sems );
		m_submit_info.setWaitDstStageMask( wait_stages );

		m_submit_info.commandBufferCount = 1;
		m_submit_info.pCommandBuffers = &( **buffers );

		std::vector< vk::Semaphore > signaled_semaphores { m_render_finished_sem[ m_current_frame_index ] };
		m_submit_info.setSignalSemaphores( signaled_semaphores );

		Device::getInstance().device().resetFences( fences );

		std::vector< vk::SubmitInfo > submit_infos { m_submit_info };

		Device::getInstance().graphicsQueue().submit( m_submit_info, m_in_flight_fence[ m_current_frame_index ] );

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

		m_current_frame_index = static_cast< FrameIndex >(
			( m_current_frame_index + static_cast< FrameIndex >( 1 ) ) % constants::MAX_FRAMES_IN_FLIGHT );

		return vk::Result::eSuccess;
	}

	void PresentSwapChain::
		transitionImages( const CommandBuffer& command_buffer, const StageID stage_id, const FrameIndex frame_index )
	{
		switch ( stage_id )
		{
			default:
				throw std::runtime_error( "Invalid StageID" );
			case INITAL:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_render_attachments.m_color.getImage( frame_index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
						m_render_attachments.m_depth.getImage( frame_index )
							.transitionTo(
								vk::ImageLayout::eUndefined,
								vk::ImageLayout::eDepthStencilAttachmentOptimal,
								vk::ImageAspectFlagBits::eDepth )
					};

					command_buffer->pipelineBarrier(
						vk::PipelineStageFlagBits::eTopOfPipe,
						vk::PipelineStageFlagBits::eColorAttachmentOutput
							| vk::PipelineStageFlagBits::eEarlyFragmentTests
							| vk::PipelineStageFlagBits::eLateFragmentTests,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
			case FINAL:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_render_attachments.m_color.getImage( frame_index )
							.transitionColorTo(
								vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR ),
					};

					command_buffer->pipelineBarrier(
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eBottomOfPipe,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
		}
	}

	vk::raii::SwapchainKHR PresentSwapChain::createSwapChain()
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

		createInfo.oldSwapchain = m_old_swap_chain == nullptr ? VK_NULL_HANDLE : *m_old_swap_chain->m_swapchain;

		return Device::getInstance()->createSwapchainKHR( createInfo );
	}

	std::vector< std::shared_ptr< Image > > PresentSwapChain::createSwapchainImages()
	{
		std::vector< vk::Image > swap_chain_images { m_swapchain.getImages() };
		std::vector< std::shared_ptr< Image > > images {};
		images.reserve( swap_chain_images.size() );

		for ( std::uint64_t i = 0; i < swap_chain_images.size(); i++ )
		{
			auto& itter = images.emplace_back(
				std::make_shared< Image >(
					m_swapchain_extent,
					m_surface_format.format,
					swap_chain_images[ i ],
					vk::ImageUsageFlagBits::eColorAttachment ) );

			itter->setName( "Swapchain image: " + std::to_string( i ) );
		}

		return images;
	}

	void PresentSwapChain::createSyncObjects()
	{
		ZoneScoped;
		m_image_available_sem.reserve( imageCount() );
		m_render_finished_sem.reserve( imageCount() );
		m_in_flight_fence.reserve( imageCount() );
		m_images_in_flight.resize( imageCount(), VK_NULL_HANDLE );

		constexpr vk::SemaphoreCreateInfo semaphoreInfo {};

		vk::FenceCreateInfo fenceInfo {};
		fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for ( size_t i = 0; i < constants::MAX_FRAMES_IN_FLIGHT; i++ )
		{
			auto& device { Device::getInstance() };

			m_image_available_sem.push_back( device->createSemaphore( semaphoreInfo ) );
			m_render_finished_sem.push_back( device->createSemaphore( semaphoreInfo ) );
			m_in_flight_fence.push_back( device->createFence( fenceInfo ) );
		}
	}

	vk::SurfaceFormatKHR PresentSwapChain::
		chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& available_formats )
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

	vk::PresentModeKHR PresentSwapChain::chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& present_modes )
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

	vk::Extent2D PresentSwapChain::chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const
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

	std::vector< vk::raii::Fence >& PresentSwapChain::getFrameFences()
	{
		return m_in_flight_fence;
	}

	PresentSwapChain::~PresentSwapChain()
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

	vk::RenderingInfo PresentSwapChain::getRenderingInfo( const FrameIndex frame_index )
	{
		static thread_local std::vector< vk::RenderingAttachmentInfo > color_info {};
		static thread_local vk::RenderingAttachmentInfo depth_info {};

		color_info = {
			m_render_attachments.m_color.renderInfo( frame_index, vk::ImageLayout::eColorAttachmentOptimal )
		};
		depth_info = m_render_attachments.m_depth.renderInfo( frame_index, vk::ImageLayout::eDepthAttachmentOptimal );

		vk::RenderingInfo info {};

		info.setColorAttachments( color_info );
		info.setPDepthAttachment( &depth_info );

		info.layerCount = 1;

		info.setRenderArea( { { 0, 0 }, m_swapchain_extent } );

		return info;
	}

	bool PresentSwapChain::compareSwapFormats( const PresentSwapChain& other ) const
	{
		return m_swap_chain_depth_format == other.m_swap_chain_depth_format
		    && m_swap_chain_format == other.m_swap_chain_format;
	}

	float PresentSwapChain::extentAspectRatio() const
	{
		return static_cast< float >( m_swapchain_extent.width ) / static_cast< float >( m_swapchain_extent.height );
	}

} // namespace fgl::engine
