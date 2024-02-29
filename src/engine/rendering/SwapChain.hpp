#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/image/Image.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace fgl::engine
{

	class SwapChain
	{
	  public:

		static constexpr std::uint16_t MAX_FRAMES_IN_FLIGHT = 2;

	  private:

		vk::Format m_swap_chain_format { vk::Format::eUndefined };
		vk::Format m_swap_chain_depth_format { findDepthFormat() };
		vk::Extent2D m_swap_chain_extent { 0, 0 };

		std::vector< vk::Framebuffer > m_swap_chain_buffers {};
		vk::RenderPass m_render_pass { VK_NULL_HANDLE };
		std::unique_ptr< RenderPassResources > m_render_pass_resources { nullptr };

		std::vector< Image > m_swap_chain_images {};

		vk::Extent2D windowExtent;

		vk::SwapchainKHR swapChain { VK_NULL_HANDLE };
		std::shared_ptr< SwapChain > old_swap_chain {};

		std::vector< vk::Semaphore > imageAvailableSemaphores {};
		std::vector< vk::Semaphore > renderFinishedSemaphores {};
		std::vector< vk::Fence > inFlightFences {};
		std::vector< vk::Fence > imagesInFlight {};
		size_t currentFrame { 0 };

		std::vector< vk::ClearValue > m_clear_values {};

		void init();
		void createSwapChain();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats );
		vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& availablePresentModes );
		vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities );

		std::array< std::unique_ptr< DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT > m_gbuffer_descriptor_set {};

	  public:

		std::vector< vk::ClearValue > getClearValues() const { return m_clear_values; }

		DescriptorSet& getGBufferDescriptor( std::uint16_t frame_idx ) const
		{
			assert( frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT && "Frame index out of range" );
			assert(
				m_gbuffer_descriptor_set.size() == SwapChain::MAX_FRAMES_IN_FLIGHT
				&& "GBuffer descriptor set not initialized" );
			return *m_gbuffer_descriptor_set[ frame_idx ];
		}

		SwapChain( vk::Extent2D windowExtent );
		SwapChain( vk::Extent2D windowExtent, std::shared_ptr< SwapChain > previous );
		~SwapChain();

		SwapChain( const SwapChain& ) = delete;
		SwapChain& operator=( const SwapChain& ) = delete;

		vk::Framebuffer getFrameBuffer( std::uint32_t index ) const
		{
			return m_swap_chain_buffers[ static_cast< std::size_t >( index ) ];
		}

		vk::RenderPass getRenderPass() const { return m_render_pass; }

		std::uint16_t imageCount() const { return static_cast< std::uint16_t >( m_swap_chain_images.size() ); }

		vk::Format getSwapChainImageFormat() const { return m_swap_chain_format; }

		vk::Extent2D getSwapChainExtent() const { return m_swap_chain_extent; }

		uint32_t width() const { return m_swap_chain_extent.width; }

		uint32_t height() const { return m_swap_chain_extent.height; }

		bool compareSwapFormats( const SwapChain& other ) const
		{
			return m_swap_chain_depth_format == other.m_swap_chain_depth_format
			    && m_swap_chain_format == other.m_swap_chain_format;
		}

		float extentAspectRatio() const
		{
			return static_cast< float >( m_swap_chain_extent.width )
			     / static_cast< float >( m_swap_chain_extent.height );
		}

		vk::Format findDepthFormat();

		[[nodiscard]] std::pair< vk::Result, std::uint32_t > acquireNextImage();
		[[nodiscard]] vk::Result submitCommandBuffers( const vk::CommandBuffer* buffers, std::uint32_t imageIndex );
	};

} // namespace fgl::engine
