#pragma once

#include <memory>
#include <vector>

#include "Attachment.hpp"
#include "Device.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/texture/Texture.hpp"
#include "types.hpp"

namespace fgl::engine
{
	class SwapChain
	{
	  public:

		static constexpr FrameIndex MAX_FRAMES_IN_FLIGHT { 2 };

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
			ColoredPresentAttachment< 0 > color;
			DepthAttachment< 1 > depth;
			InputColorAttachment< 2 > input_color { vk::Format::eR8G8B8A8Unorm };
		} render_attachments;

		vk::raii::RenderPass m_render_pass;

		std::vector< vk::raii::Framebuffer > m_swap_chain_buffers;

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > m_input_descriptors {};

		std::vector< vk::ClearValue > m_clear_values;

		std::vector< vk::raii::Semaphore > image_available_sem {};
		std::vector< vk::raii::Semaphore > render_finished_sem {};
		std::vector< vk::raii::Fence > in_flight_fence {};
		std::vector< vk::Fence > images_in_flight {};

		FrameIndex m_current_frame_index { 0 };

		void init();
		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain();
		[[nodiscard]] std::vector< Image > createSwapchainImages();
		[[nodiscard]] vk::raii::RenderPass createRenderPass();
		[[nodiscard]] std::vector< vk::raii::Framebuffer > createFramebuffers();
		[[nodiscard]] std::vector< std::unique_ptr< descriptors::DescriptorSet > > createInputDescriptors();
		void createSyncObjects();

		// Helper functions
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >&
		                                                         available_formats );
		static vk::PresentModeKHR chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& present_modes );
		vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const;

	  public:

		SwapChain( vk::Extent2D windowExtent, PhysicalDevice& phy_dev );
		SwapChain( vk::Extent2D windowExtent, std::shared_ptr< SwapChain > previous );

		SwapChain( const SwapChain& ) = delete;
		SwapChain& operator=( const SwapChain& ) = delete;

		~SwapChain();

		descriptors::DescriptorSet& getInputDescriptor( const PresentIndex present_index );

		Image& getInputImage( PresentIndex present_index ) const;

		const std::vector< vk::ClearValue >& getClearValues() const { return m_clear_values; }

		vk::raii::Framebuffer& getFrameBuffer( PresentIndex present_index );

		vk::raii::RenderPass& getRenderPass() { return m_render_pass; }

		PresentIndex imageCount() const { return static_cast< std::uint16_t >( m_swap_chain_images.size() ); }

		vk::Format getSwapChainImageFormat() const { return m_swap_chain_format; }

		vk::Extent2D getSwapChainExtent() const { return m_swapchain_extent; }

		uint32_t width() const { return m_swapchain_extent.width; }

		uint32_t height() const { return m_swapchain_extent.height; }

		bool compareSwapFormats( const SwapChain& other ) const;

		float extentAspectRatio() const;

		static vk::Format findDepthFormat();

		[[nodiscard]] std::pair< vk::Result, PresentIndex > acquireNextImage();
		[[nodiscard]] vk::Result
			submitCommandBuffers( const vk::raii::CommandBuffer& buffers, PresentIndex present_index );
	};

	template < typename T >
	using PerFrameArray = std::array< T, SwapChain::MAX_FRAMES_IN_FLIGHT >;

} // namespace fgl::engine
