#pragma once

#include "Device.hpp"

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
		VkFormat m_swap_chain_format;
		VkFormat m_swap_chain_depth_format;
		VkExtent2D m_swap_chain_extent;

		std::vector< VkFramebuffer > m_swap_chain_buffers;
		VkRenderPass m_render_pass;

		std::vector< VkImage > m_depth_images;
		std::vector< VkDeviceMemory > m_depth_memory;
		std::vector< VkImageView > m_depth_image_views;
		std::vector< VkImage > m_swap_chain_images;
		std::vector< VkImageView > m_swap_chain_views;

		Device& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr< SwapChain > old_swap_chain;

		std::vector< VkSemaphore > imageAvailableSemaphores;
		std::vector< VkSemaphore > renderFinishedSemaphores;
		std::vector< VkFence > inFlightFences;
		std::vector< VkFence > imagesInFlight;
		size_t currentFrame { 0 };

		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector< VkSurfaceFormatKHR >& availableFormats );
		VkPresentModeKHR chooseSwapPresentMode( const std::vector< VkPresentModeKHR >& availablePresentModes );
		VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities );

	  public:

		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		SwapChain( Device& deviceRef, VkExtent2D windowExtent );
		SwapChain( Device& deviceRef, VkExtent2D windowExtent, std::shared_ptr< SwapChain > previous );
		~SwapChain();

		SwapChain( const SwapChain& ) = delete;
		SwapChain& operator=( const SwapChain& ) = delete;

		VkFramebuffer getFrameBuffer( int index ) const { return m_swap_chain_buffers[ index ]; }

		VkRenderPass getRenderPass() const { return m_render_pass; }

		VkImageView getImageView( int index ) const { return m_swap_chain_views[ index ]; }

		size_t imageCount() const { return m_swap_chain_images.size(); }

		VkFormat getSwapChainImageFormat() const { return m_swap_chain_format; }

		VkExtent2D getSwapChainExtent() const { return m_swap_chain_extent; }

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

		VkFormat findDepthFormat();

		VkResult acquireNextImage( uint32_t* imageIndex );
		VkResult submitCommandBuffers( const VkCommandBuffer* buffers, uint32_t* imageIndex );
	};

} // namespace fgl::engine
