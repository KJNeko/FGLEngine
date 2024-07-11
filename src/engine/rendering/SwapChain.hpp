#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include "Device.hpp"
#include "RenderPassBuilder.hpp"
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

		SwapChainSupportDetails m_swapchain_details;
		vk::SurfaceFormatKHR m_surface_format;
		vk::PresentModeKHR m_present_mode;
		vk::Extent2D m_swapchain_extent;

		vk::Format m_swap_chain_format;
		vk::Format m_swap_chain_depth_format;

		std::shared_ptr< SwapChain > old_swap_chain;

		vk::raii::SwapchainKHR m_swapchain;
		std::vector< Image > m_swap_chain_images;

		struct
		{
			ColoredPresentAttachment color;
			DepthAttachment depth;
		} render_attachments;

		struct
		{
			ColorAttachment position { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment normal { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment albedo { vk::Format::eR8G8B8A8Unorm };
			ColorAttachment composite { vk::Format::eR8G8B8A8Unorm };
		} gbuffer {};

	  public:

		std::unique_ptr< Texture > g_buffer_position_img { nullptr };
		std::unique_ptr< Texture > g_buffer_normal_img { nullptr };
		std::unique_ptr< Texture > g_buffer_albedo_img { nullptr };
		std::unique_ptr< Texture > g_buffer_composite_img { nullptr };

	  private:

		RenderPassBuilder render_pass_builder {};

		vk::raii::RenderPass m_render_pass;

		std::vector< vk::raii::Framebuffer > m_swap_chain_buffers;

		std::vector< vk::ClearValue > m_clear_values;

		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_gbuffer_descriptor_set;

		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_gbuffer_composite_descriptor_set;

		std::vector< vk::raii::Semaphore > image_available_sem {};
		std::vector< vk::raii::Semaphore > render_finished_sem {};
		std::vector< vk::raii::Fence > in_flight_fence {};
		std::vector< vk::Fence > images_in_flight {};
		size_t m_current_frame_index { 0 };

		void init();
		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain();
		[[nodiscard]] std::vector< Image > createSwapchainImages();
		[[nodiscard]] vk::raii::RenderPass createRenderPass();
		[[nodiscard]] std::vector< vk::raii::Framebuffer > createFramebuffers();
		void createSyncObjects();

		// Helper functions
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& available_formats );
		static vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& present_modes );
		vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const;

		template < is_attachment... Attachments >
		static std::vector< vk::ImageView > getViewsForFrame( const std::uint8_t frame_idx, Attachments... attachments )
		{
			std::vector< vk::ImageView > view {};
			view.resize( sizeof...( Attachments ) );

			( ( view[ attachments.getIndex() ] = *attachments.getView( frame_idx ) ), ... );

			return view;
		}

		template < is_attachment... Attachments >
		static std::vector< vk::ClearValue > gatherClearValues( Attachments... attachments )
		{
			std::vector< vk::ClearValue > clear_values {};
			clear_values.resize( sizeof...( Attachments ) );

			( ( clear_values[ attachments.getIndex() ] = attachments.m_clear_value ), ... );

			return clear_values;
		}

		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			createGBufferDescriptors();
		std::array< std::unique_ptr< descriptors::DescriptorSet >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			createCompositeDescriptors();

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

		vk::Extent2D getSwapChainExtent() const { return m_swapchain_extent; }

		uint32_t width() const { return m_swapchain_extent.width; }

		uint32_t height() const { return m_swapchain_extent.height; }

		bool compareSwapFormats( const SwapChain& other ) const
		{
			return m_swap_chain_depth_format == other.m_swap_chain_depth_format
			    && m_swap_chain_format == other.m_swap_chain_format;
		}

		float extentAspectRatio() const
		{
			return static_cast< float >( m_swapchain_extent.width ) / static_cast< float >( m_swapchain_extent.height );
		}

		static vk::Format findDepthFormat();

		[[nodiscard]] std::pair< vk::Result, std::uint32_t > acquireNextImage();
		[[nodiscard]] vk::Result
			submitCommandBuffers( const vk::raii::CommandBuffer& buffers, std::uint32_t imageIndex );
	};

} // namespace fgl::engine
