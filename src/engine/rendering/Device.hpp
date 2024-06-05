#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <string>
#include <vector>

#include "engine/Window.hpp"
#include "engine/concepts/is_suballocation.hpp"
#include "vma/vma_impl.hpp"

namespace fgl::engine
{

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities {};
		std::vector< vk::SurfaceFormatKHR > formats {};
		std::vector< vk::PresentModeKHR > presentModes {};
	};

	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily { 0 };
		uint32_t presentFamily { 0 };
		bool graphicsFamilyHasValue { false };
		bool presentFamilyHasValue { false };

		bool isComplete() const { return graphicsFamilyHasValue && presentFamilyHasValue; }
	};

	class Device
	{
		vk::Instance m_instance { VK_NULL_HANDLE };
		vk::DebugUtilsMessengerEXT m_debugMessenger { VK_NULL_HANDLE };
		vk::PhysicalDevice m_physical_device { VK_NULL_HANDLE };
		vk::CommandPool m_commandPool { VK_NULL_HANDLE };

		VmaAllocator m_allocator { VK_NULL_HANDLE };

		vk::Device m_device { VK_NULL_HANDLE };
		vk::SurfaceKHR m_surface_khr { VK_NULL_HANDLE };
		vk::Queue m_graphics_queue { VK_NULL_HANDLE };
		vk::Queue m_present_queue { VK_NULL_HANDLE };

		std::vector< const char* > validationLayers { "VK_LAYER_KHRONOS_validation" };
		std::vector< const char* > deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			                                          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

		void copyBuffer(
			vk::Buffer dst,
			vk::Buffer src,
			vk::DeviceSize dst_offset,
			vk::DeviceSize src_offset,
			vk::DeviceSize size = VK_WHOLE_SIZE );

	  public:

		vk::PhysicalDeviceProperties m_properties {};

		static Device& init( Window& window );
		static Device& getInstance();

		template < typename Dst, typename Src >
			requires( is_buffer< Dst > || is_suballocation< Dst > ) && (is_buffer< Src > || is_suballocation< Src >)
		void copyBuffer(
			Dst& dst,
			Src& src,
			vk::DeviceSize dst_offset,
			vk::DeviceSize src_offset,
			vk::DeviceSize size = VK_WHOLE_SIZE )
		{
			copyBuffer( dst.getBuffer(), src.getBuffer(), dst_offset, src_offset, size );
		}

		template < typename Dst, typename Src >
			requires is_suballocation< Dst > && is_suballocation< Src >
		void copyBuffer( Dst& dst, Src& src, vk::DeviceSize size )
		{
			copyBuffer( dst, src, dst.offset(), src.offset(), size );
		}

	  private:

		void createInstance();
		void setupDebugMessenger();
		void createSurface( Window& window );
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createVMAAllocator();
		void createCommandPool();

		// helper functions
		bool isDeviceSuitable( vk::PhysicalDevice device );
		std::vector< const char* > getRequiredInstanceExtensions();
		bool checkValidationLayerSupport();
		QueueFamilyIndices findQueueFamilies( vk::PhysicalDevice device );
		void populateDebugMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& createInfo );
		void hasGlfwRequiredInstanceExtensions();
		bool checkDeviceExtensionSupport( vk::PhysicalDevice device );
		SwapChainSupportDetails querySwapChainSupport( vk::PhysicalDevice device );

	  public:

#ifndef NDEBUG
		const bool enableValidationLayers { true };
#else
		const bool enableValidationLayers { false };
#endif

		vk::Result setDebugUtilsObjectName( const vk::DebugUtilsObjectNameInfoEXT& nameInfo );

	  public:

		Device( Window& window );
		~Device();

		// Not copyable or movable
		Device( const Device& ) = delete;
		Device& operator=( const Device& ) = delete;
		Device( Device&& ) = delete;
		Device& operator=( Device&& ) = delete;

		vk::CommandPool getCommandPool() { return m_commandPool; }

		vk::Device device() { return m_device; }

		vk::Instance instance() { return m_instance; }

		vk::PhysicalDevice phyDevice() { return m_physical_device; }

		vk::SurfaceKHR surface() { return m_surface_khr; }

		vk::Queue graphicsQueue() { return m_graphics_queue; }

		vk::Queue presentQueue() { return m_present_queue; }

		VmaAllocator allocator() { return m_allocator; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport( m_physical_device ); }

		uint32_t findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags properties );

		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies( m_physical_device ); }

		vk::Format findSupportedFormat(
			const std::vector< vk::Format >& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );

		vk::CommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands( vk::CommandBuffer commandBuffer );

		void copyBufferToImage(
			vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount );
	};

} // namespace fgl::engine