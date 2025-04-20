#pragma once

#include <memory>
#include <vector>

#include "devices/Device.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "pipelines/Attachment.hpp"
#include "types.hpp"

namespace fgl::engine
{
	class PresentSwapChain
	{
	  public:

		enum StageID
		{
			INITAL,
			FINAL
		};

	  private:

		PhysicalDevice& m_phy_device;

		SwapChainSupportDetails m_swapchain_details;
		vk::SurfaceFormatKHR m_surface_format;
		vk::PresentModeKHR m_present_mode;
		vk::Extent2D m_swapchain_extent;

		vk::Format m_swap_chain_format;
		vk::Format m_swap_chain_depth_format;

		std::shared_ptr< PresentSwapChain > m_old_swap_chain;

		vk::raii::SwapchainKHR m_swapchain;
		std::vector< std::shared_ptr< Image > > m_swap_chain_images;

		struct
		{
			ColoredPresentAttachment< 0 > m_color;
			DepthAttachment< 1 > m_depth;
		} m_render_attachments { m_swap_chain_format, m_swap_chain_depth_format };

		std::vector< descriptors::DescriptorSetPtr > m_input_descriptors {};

		std::vector< vk::ClearValue > m_clear_values;

		std::vector< vk::raii::Semaphore > m_image_available_sem {};
		std::vector< vk::raii::Semaphore > m_render_finished_sem {};
		std::vector< vk::raii::Fence > m_in_flight_fence {};
		std::vector< vk::Fence > m_images_in_flight {};

		FrameIndex m_current_frame_index { 0 };

		void init();
		[[nodiscard]] vk::raii::SwapchainKHR createSwapChain();
		[[nodiscard]] std::vector< std::shared_ptr< Image > > createSwapchainImages();
		[[nodiscard]] std::vector< descriptors::DescriptorSetPtr > createInputDescriptors();
		void createSyncObjects();

		// Helper functions
		[[nodiscard]] static vk::SurfaceFormatKHR
			chooseSwapSurfaceFormat( const std::vector< vk::SurfaceFormatKHR >& available_formats );
		[[nodiscard]] static vk::PresentModeKHR
			chooseSwapPresentMode( const std::vector< vk::PresentModeKHR >& present_modes );
		[[nodiscard]] vk::Extent2D chooseSwapExtent( const vk::SurfaceCapabilitiesKHR& capabilities ) const;

	  public:

		std::vector< vk::raii::Fence >& getFrameFences();

		PresentSwapChain( vk::Extent2D windowExtent, PhysicalDevice& phy_dev );
		PresentSwapChain( vk::Extent2D windowExtent, std::shared_ptr< PresentSwapChain > previous );

		PresentSwapChain( const PresentSwapChain& ) = delete;
		PresentSwapChain& operator=( const PresentSwapChain& ) = delete;

		~PresentSwapChain();

		[[nodiscard]] vk::RenderingInfo getRenderingInfo( const FrameIndex frame_index );

		// Image& getInputImage( PresentIndex present_index ) const;

		[[nodiscard]] const std::vector< vk::ClearValue >& getClearValues() const { return m_clear_values; }

		[[nodiscard]] PresentIndex imageCount() const
		{
			return static_cast< std::uint16_t >( m_swap_chain_images.size() );
		}

		[[nodiscard]] vk::Extent2D getSwapChainExtent() const { return m_swapchain_extent; }

		[[nodiscard]] uint32_t width() const { return m_swapchain_extent.width; }

		[[nodiscard]] uint32_t height() const { return m_swapchain_extent.height; }

		[[nodiscard]] bool compareSwapFormats( const PresentSwapChain& other ) const;

		[[nodiscard]] float extentAspectRatio() const;

		[[nodiscard]] std::pair< vk::Result, PresentIndex > acquireNextImage();
		[[nodiscard]] vk::Result submitCommandBuffers( const CommandBuffer& buffers, PresentIndex present_index );

		void transitionImages( const CommandBuffer& command_buffer, StageID stage_id, FrameIndex frame_index ) const;
	};

	template < typename T >
	using PerFrameArray = std::array< T, constants::MAX_FRAMES_IN_FLIGHT >;

	template < typename T, typename... TArgs >
	PerFrameArray< T > constructPerFrame( TArgs&... args )
	{
		//TODO: Figure this out
		return { { args..., args... } };
	}

} // namespace fgl::engine
