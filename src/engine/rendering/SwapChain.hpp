#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "Device.hpp"
#include "RenderPass.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine
{
	class SwapChain
	{
	  public:

		static constexpr std::uint16_t MAX_FRAMES_IN_FLIGHT = 2;

	  private:

		PhysicalDevice& m_phy_device;

		vk::Format m_swap_chain_format { vk::Format::eUndefined };
		vk::Format m_swap_chain_depth_format { findDepthFormat() };
		vk::Extent2D m_swap_chain_extent { 0, 0 };

		std::vector< vk::raii::Framebuffer > m_swap_chain_buffers {};
		vk::raii::RenderPass m_render_pass { VK_NULL_HANDLE };
		std::unique_ptr< RenderPassResources > m_render_pass_resources { nullptr };

		std::vector< Image > m_swap_chain_images {};

		vk::Extent2D windowExtent;

		vk::raii::SwapchainKHR swapChain { VK_NULL_HANDLE };
		std::shared_ptr< SwapChain > old_swap_chain {};

		std::vector< vk::raii::Semaphore > imageAvailableSemaphores {};
		std::vector< vk::raii::Semaphore > renderFinishedSemaphores {};
		std::vector< vk::raii::Fence > inFlightFences {};
		std::vector< vk::Fence > imagesInFlight {};
		size_t currentFrame { 0 };

		std::vector< vk::ClearValue > m_clear_values {};

	  public:

		std::unique_ptr< Texture > g_buffer_position_img { nullptr };
		std::unique_ptr< Texture > g_buffer_normal_img { nullptr };
		std::unique_ptr< Texture > g_buffer_albedo_img { nullptr };
		std::unique_ptr< Texture > g_buffer_composite_img { nullptr };

	  private:

		void init();
		void createSwapChain();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& availableFormats );
		vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& availablePresentModes );
		vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities );

		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_gbuffer_descriptor_set {};

		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_gbuffer_composite_descriptor_set {};

	  public:

		std::vector< vk::ClearValue > getClearValues() const { return m_clear_values; }

		descriptors::DescriptorSet& getGBufferDescriptor( std::uint16_t frame_idx ) const
		{
			assert( frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT && "Frame index out of range" );
			assert(
				m_gbuffer_descriptor_set.size() == SwapChain::MAX_FRAMES_IN_FLIGHT
				&& "GBuffer descriptor set not initialized" );
			return *m_gbuffer_descriptor_set[ frame_idx ];
		}

		descriptors::DescriptorSet& getGBufferCompositeDescriptor( uint16_t frame_idx ) const
		{
			assert( frame_idx < SwapChain::MAX_FRAMES_IN_FLIGHT && "Frame index out of range" );

			return *m_gbuffer_composite_descriptor_set[ frame_idx ];
		}

		SwapChain( vk::Extent2D windowExtent, PhysicalDevice& phy_dev );
		SwapChain( vk::Extent2D windowExtent, std::shared_ptr< SwapChain > previous );

		SwapChain( const SwapChain& ) = delete;
		SwapChain& operator=( const SwapChain& ) = delete;

		vk::raii::Framebuffer& getFrameBuffer( std::uint32_t index )
		{
			return m_swap_chain_buffers[ static_cast< std::size_t >( index ) ];
		}

		vk::raii::RenderPass& getRenderPass() { return m_render_pass; }

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
		[[nodiscard]] vk::Result
			submitCommandBuffers( const vk::raii::CommandBuffer& buffers, std::uint32_t imageIndex );
	};

} // namespace fgl::engine
