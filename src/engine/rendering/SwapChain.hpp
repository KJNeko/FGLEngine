#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "Device.hpp"
#include "RenderPassBuilder.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/rendering/types.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine
{
	class SwapChain
	{
	  public:

		static constexpr FrameIndex MAX_FRAMES_IN_FLIGHT { 2 };

	  private:

		PhysicalDevice& m_phy_device;
		std::shared_ptr< SwapChain > old_swap_chain {};

		SwapChainSupportDetails m_swapchain_support;
		vk::SurfaceFormatKHR m_surface_format;

		vk::Format m_swap_chain_format { vk::Format::eUndefined };
		vk::Format m_swap_chain_depth_format { findDepthFormat() };
		vk::Extent2D m_swap_chain_extent { 0, 0 };

		std::vector< std::vector< vk::raii::Framebuffer > > m_swap_chain_buffers {};
		vk::raii::RenderPass m_render_pass { VK_NULL_HANDLE };

		std::vector< Image > m_swap_chain_images {};

		vk::Extent2D windowExtent;

		vk::raii::SwapchainKHR swapChain { VK_NULL_HANDLE };

		std::vector< vk::raii::Semaphore > imageAvailableSemaphores {};
		std::vector< vk::raii::Semaphore > renderFinishedSemaphores {};
		std::vector< vk::raii::Fence > in_flight_fences {};
		std::vector< vk::Fence > images_in_flight {};
		size_t current_frame_index { 0 };

		std::vector< vk::ClearValue > m_clear_values {};

		//! Attachments for the final render target
		struct
		{
			ColoredPresentAttachment color; // Present attachment
			DepthAttachment depth;
		} m_render_attachments;

		struct
		{
			ColorAttachment position { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment normal { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment albedo { vk::Format::eR8G8B8A8Unorm };
			ColorAttachment composite { vk::Format::eR8G8B8A8Unorm };
		} m_gbuffer_attachments {};

		void init();
		void createSwapChain();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		template < is_attachment... Attachments >
		void populateAttachmentClearValues( Attachments&... attachments )
		{
			m_clear_values.resize( sizeof...( Attachments ) );
			( ( m_clear_values[ attachments.getIndex() ] = attachments.m_clear_value ), ... );
		}

		// Helper functions
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& available_formats );
		vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& available_present_modes );
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

		vk::raii::Framebuffer& getFrameBuffer( const FrameIndex frame_index, const std::uint16_t present_idx );

		vk::raii::RenderPass& getRenderPass() { return m_render_pass; }

		std::uint16_t imageCount() const { return m_swap_chain_images.size(); }

		vk::Format getSwapChainImageFormat() const
		{
			assert( m_swap_chain_format != vk::Format::eUndefined );
			return m_swap_chain_format;
		}

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
			submitCommandBuffers( const vk::raii::CommandBuffer& buffers, PresentIndex current_present_index );
	};

} // namespace fgl::engine
