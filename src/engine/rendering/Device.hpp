#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <string>
#include <vector>

#include "Instance.hpp"
#include "PhysicalDevice.hpp"
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
		Instance& m_instance;

		Surface m_surface_khr;

		PhysicalDevice m_physical_device;

		inline static std::vector< const char* > validationLayers { "VK_LAYER_KHRONOS_validation" };
		inline static std::vector< const char* > deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			                                                        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

		struct DeviceCreateInfo
		{
			vk::PhysicalDeviceFeatures m_requested_features;
			vk::PhysicalDeviceDescriptorIndexingFeatures m_indexing_features;
			std::vector< vk::DeviceQueueCreateInfo > m_queue_create_infos;
			vk::DeviceCreateInfo m_create_info;

			vk::PhysicalDeviceFeatures getDeviceFeatures( PhysicalDevice& );
			vk::PhysicalDeviceDescriptorIndexingFeatures getIndexingFeatures();
			std::vector< vk::DeviceQueueCreateInfo > getQueueCreateInfos( PhysicalDevice& );
			vk::DeviceCreateInfo getCreateInfo( PhysicalDevice& );

			DeviceCreateInfo( PhysicalDevice& );

		} device_creation_info;

		vk::raii::Device m_device;

		vk::raii::CommandPool m_commandPool;

		vk::raii::Queue m_graphics_queue;
		vk::raii::Queue m_present_queue;

		VmaAllocator m_allocator;

	  public:

		vk::PhysicalDeviceProperties m_properties;

		vk::CommandPoolCreateInfo commandPoolInfo();

		Device( Window&, Instance& );
		~Device();

		static Device& getInstance();

	  private:

		VmaAllocator createVMAAllocator();

		// helper functions
		bool checkValidationLayerSupport();
		bool checkDeviceExtensionSupport( vk::raii::PhysicalDevice device );
		SwapChainSupportDetails querySwapChainSupport( vk::raii::PhysicalDevice device );

	  public:

#ifndef NDEBUG
		const bool enableValidationLayers { true };
#else
		const bool enableValidationLayers { false };
#endif

		vk::Result setDebugUtilsObjectName( const vk::DebugUtilsObjectNameInfoEXT& nameInfo );

	  public:

		// Not copyable or movable
		FGL_DELETE_DEFAULT_CTOR( Device )
		FGL_DELETE_COPY( Device )
		FGL_DELETE_MOVE( Device )

		vk::CommandPool getCommandPool() { return m_commandPool; }

		vk::raii::Device& device() { return m_device; }

		Instance& instance() { return m_instance; }

		PhysicalDevice& phyDevice() { return m_physical_device; }

		vk::SurfaceKHR surface() { return m_surface_khr; }

		vk::raii::Queue& graphicsQueue() { return m_graphics_queue; }

		vk::raii::Queue& presentQueue() { return m_present_queue; }

		VmaAllocator allocator() { return m_allocator; }

		SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport( m_physical_device ); }

		uint32_t findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags properties );

		vk::Format findSupportedFormat(
			const std::vector< vk::Format >& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );

		vk::raii::CommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands( vk::raii::CommandBuffer& commandBuffer );

		void copyBufferToImage(
			vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount );

		VkDevice operator*() { return *m_device; }

		vk::raii::Device* operator->() { return &m_device; }
	};

} // namespace fgl::engine