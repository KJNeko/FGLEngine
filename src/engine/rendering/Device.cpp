#include "Device.hpp"

#include "engine/descriptors/DescriptorPool.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

#include "engine/logging/logging.hpp"

namespace fgl::engine
{

	Device* global_device { nullptr };

	Device& Device::getInstance()
	{
		assert( global_device && "Device not initialized" );
		return *global_device;
	}

	// class member functions
	Device::Device( Window& window, Instance& instance ) : m_instance( instance )
	{
		assert( !global_device );

		createSurface( window );
		pickPhysicalDevice();
		createLogicalDevice();
		createVMAAllocator();
		createCommandPool();

		global_device = this;

		DescriptorPool::init( *global_device );
	}

	Device::~Device()
	{
		vkDestroyCommandPool( m_device, m_commandPool, nullptr );
		vkDestroyDevice( m_device, nullptr );

		vkDestroySurfaceKHR( m_instance, m_surface_khr, nullptr );
		vkDestroyInstance( m_instance, nullptr );
	}

	void Device::pickPhysicalDevice()
	{
		std::vector< vk::PhysicalDevice > devices {
			static_cast< vk::Instance >( m_instance ).enumeratePhysicalDevices()
		};

		bool found { false };

		for ( const auto& device : devices )
		{
			if ( isDeviceSuitable( device ) )
			{
				m_physical_device = device;
				found = true;
				break;
			}
		}

		m_properties = m_physical_device.getProperties();

		if ( !found )
		{
			throw std::runtime_error( "failed to find a suitable GPU!" );
		}
	}

	void Device::createLogicalDevice()
	{
		const QueueFamilyIndices indices { findQueueFamilies( m_physical_device ) };

		std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos;
		std::set< uint32_t > uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for ( uint32_t queueFamily : uniqueQueueFamilies )
		{
			vk::DeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back( queueCreateInfo );
		}

		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.multiDrawIndirect = VK_TRUE;
		deviceFeatures.tessellationShader = VK_TRUE;
		deviceFeatures.drawIndirectFirstInstance = VK_TRUE;

		vk::PhysicalDeviceDescriptorIndexingFeatures indexing_features {};
		indexing_features.setRuntimeDescriptorArray( true );
		indexing_features.setDescriptorBindingPartiallyBound( true );
		indexing_features.setShaderSampledImageArrayNonUniformIndexing( true );
		indexing_features.setDescriptorBindingSampledImageUpdateAfterBind( true );

		vk::DeviceCreateInfo createInfo {};
		createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size() );
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast< uint32_t >( deviceExtensions.size() );
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		createInfo.setPNext( &indexing_features );

		//Get device extension list
		const auto supported_extensions { m_physical_device.enumerateDeviceExtensionProperties() };
		std::cout << "Supported device extensions:" << std::endl;
		for ( auto& desired_ext : deviceExtensions )
		{
			bool found { false };
			for ( auto& supported_ext : supported_extensions )
			{
				if ( strcmp( desired_ext, supported_ext.extensionName ) == 0 )
				{
					found = true;
					break;
				}
			}
			std::cout << "\t" << desired_ext << ": " << found << std::endl;
			if ( !found ) throw std::runtime_error( "Failed to find required extension" );
		}

		// might not really be necessary anymore because device specific validation layers
		// have been deprecated
		if ( enableValidationLayers )
		{
			createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if ( auto res = m_physical_device.createDevice( &createInfo, nullptr, &m_device ); res != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create logical device!" );
		}

		m_device.getQueue( indices.graphicsFamily, 0, &m_graphics_queue );
		m_device.getQueue( indices.presentFamily, 0, &m_present_queue );
	}

	void Device::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient;

		if ( m_device.createCommandPool( &poolInfo, nullptr, &m_commandPool ) != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create command pool!" );
		}
	}

	void Device::createSurface( Window& window )
	{
		m_surface_khr = window.createWindowSurface( m_instance );
	}

	bool Device::isDeviceSuitable( vk::PhysicalDevice device )
	{
		const QueueFamilyIndices indices { findQueueFamilies( device ) };

		const bool extensionsSupported { checkDeviceExtensionSupport( device ) };

		bool swapChainAdequate { false };
		if ( extensionsSupported )
		{
			const SwapChainSupportDetails swapChainSupport { querySwapChainSupport( device ) };
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		vk::PhysicalDeviceFeatures supportedFeatures;
		device.getFeatures( &supportedFeatures );

		std::cout << "Device: " << device.getProperties().deviceName << std::endl;
		std::cout << "\tgraphicsFamily: " << indices.graphicsFamily << std::endl;
		std::cout << "\tpresentFamily: " << indices.presentFamily << std::endl;
		std::cout << "\textensionsSupported: " << extensionsSupported << std::endl;
		std::cout << "\tswapChainAdequate: " << swapChainAdequate << std::endl;
		std::cout << "\tsamplerAnisotropy: " << supportedFeatures.samplerAnisotropy << std::endl;

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	bool Device::checkValidationLayerSupport()
	{
		std::vector< vk::LayerProperties > availableLayers { vk::enumerateInstanceLayerProperties() };

		for ( const char* layerName : validationLayers )
		{
			bool layerFound = false;

			for ( const auto& layerProperties : availableLayers )
			{
				if ( strcmp( layerName, layerProperties.layerName ) == 0 )
				{
					layerFound = true;
					break;
				}
			}

			if ( !layerFound )
			{
				return false;
			}
		}

		return true;
	}

	std::vector< const char* > Device::getRequiredInstanceExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

		if ( glfwExtensions == nullptr ) throw std::runtime_error( "Failed to get required extensions from glfw" );

		std::vector< const char* > extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

		// "VK_KHR_surface" is guaranteed to be in this list
		assert( extensions.size() >= 1 );

		if ( enableValidationLayers )
		{
			extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
		}

		return extensions;
	}

	bool Device::checkDeviceExtensionSupport( vk::PhysicalDevice device )
	{
		const std::vector< vk::ExtensionProperties > availableExtensions {
			device.enumerateDeviceExtensionProperties()
		};

		std::set< std::string > requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

		std::uint32_t required_count { static_cast< std::uint32_t >( requiredExtensions.size() ) };
		std::uint32_t found_count { 0 };

		for ( const auto required : deviceExtensions )
		{
			if ( std::find_if(
					 availableExtensions.begin(),
					 availableExtensions.end(),
					 [ required ]( const vk::ExtensionProperties& prop )
					 { return std::strcmp( prop.extensionName, required ); } )
			     != availableExtensions.end() )
				found_count++;
		}

		return found_count == required_count;
	}

	QueueFamilyIndices Device::findQueueFamilies( vk::PhysicalDevice device )
	{
		QueueFamilyIndices indices {};

		std::vector< vk::QueueFamilyProperties > queueFamilies { device.getQueueFamilyProperties() };

		int i { 0 };
		for ( const auto& queueFamily : queueFamilies )
		{
			if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics )
			{
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			vk::Bool32 presentSupport { VK_FALSE };
			vkGetPhysicalDeviceSurfaceSupportKHR( device, i, m_surface_khr, &presentSupport );
			if ( queueFamily.queueCount > 0 && presentSupport )
			{
				indices.presentFamily = i;
				indices.presentFamilyHasValue = true;
			}
			if ( indices.isComplete() )
			{
				break;
			}

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails Device::querySwapChainSupport( vk::PhysicalDevice device )
	{
		SwapChainSupportDetails details;

		if ( device.getSurfaceCapabilitiesKHR( m_surface_khr, &details.capabilities ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to get surface capabilities" );

		uint32_t formatCount { 0 };
		if ( device.getSurfaceFormatsKHR( m_surface_khr, &formatCount, nullptr ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to get surface formats" );

		if ( formatCount != 0 )
		{
			details.formats.resize( formatCount );
			if ( device.getSurfaceFormatsKHR( m_surface_khr, &formatCount, details.formats.data() )
			     != vk::Result::eSuccess )
				throw std::runtime_error( "failed to get surface formats" );
		}

		uint32_t presentModeCount { 0 };
		if ( device.getSurfacePresentModesKHR( m_surface_khr, &presentModeCount, nullptr ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to get surface present modes" );

		if ( presentModeCount != 0 )
		{
			details.presentModes.resize( presentModeCount );
			if ( device.getSurfacePresentModesKHR( m_surface_khr, &presentModeCount, details.presentModes.data() )
			     != vk::Result::eSuccess )
				throw std::runtime_error( "failed to get surface present modes" );
		}
		return details;
	}

	vk::Format Device::findSupportedFormat(
		const std::vector< vk::Format >& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features )
	{
		for ( vk::Format format : candidates )
		{
			vk::FormatProperties props;
			m_physical_device.getFormatProperties( format, &props );

			if ( tiling == vk::ImageTiling::eLinear && ( props.linearTilingFeatures & features ) == features )
			{
				return format;
			}
			else if ( tiling == vk::ImageTiling::eOptimal && ( props.optimalTilingFeatures & features ) == features )
			{
				return format;
			}
		}
		throw std::runtime_error( "failed to find supported format!" );
	}

	uint32_t Device::findMemoryType( uint32_t typeFilter, vk::MemoryPropertyFlags properties )
	{
		vk::PhysicalDeviceMemoryProperties memProperties;
		m_physical_device.getMemoryProperties( &memProperties );
		for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
		{
			if ( ( typeFilter & ( 1 << i ) )
			     && ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties )
			{
				return i;
			}
		}

		throw std::runtime_error( "failed to find suitable memory type!" );
	}

	vk::CommandBuffer Device::beginSingleTimeCommands()
	{
		ZoneScoped;
		vk::CommandBufferAllocateInfo allocInfo {};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer {};
		if ( m_device.allocateCommandBuffers( &allocInfo, &commandBuffer ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to allocate command buffers!" );

		vk::CommandBufferBeginInfo beginInfo {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer.begin( beginInfo );

		return commandBuffer;
	}

	void Device::endSingleTimeCommands( vk::CommandBuffer commandBuffer )
	{
		ZoneScoped;
		vkEndCommandBuffer( commandBuffer );

		vk::SubmitInfo submitInfo {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if ( m_graphics_queue.submit( 1, &submitInfo, VK_NULL_HANDLE ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to submit single time command buffer!" );

		m_graphics_queue.waitIdle();

		m_device.freeCommandBuffers( m_commandPool, 1, &commandBuffer );
	}

	void Device::
		copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount )
	{
		ZoneScoped;
		vk::CommandBuffer commandBuffer { beginSingleTimeCommands() };

		vk::BufferImageCopy region {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = vk::Offset3D { 0, 0, 0 };
		region.imageExtent = vk::Extent3D { width, height, 1 };

		commandBuffer.copyBufferToImage( buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region );
		endSingleTimeCommands( commandBuffer );
	}

	void Device::createVMAAllocator()
	{
		VmaVulkanFunctions vk_func {};
		vk_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vk_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo create_info {};
		create_info.physicalDevice = m_physical_device;
		create_info.device = m_device;
		create_info.pVulkanFunctions = &vk_func;
		create_info.instance = m_instance;
		create_info.vulkanApiVersion = VK_API_VERSION_1_0;

		if ( vmaCreateAllocator( &create_info, &m_allocator ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create VMA allocator" );
	}

	void Device::copyBuffer(
		vk::Buffer dst, vk::Buffer src, vk::DeviceSize dst_offset, vk::DeviceSize src_offset, vk::DeviceSize size )
	{
		vk::CommandBuffer commandBuffer { beginSingleTimeCommands() };

		vk::BufferCopy copyRegion {};
		copyRegion.size = size;
		copyRegion.srcOffset = src_offset;
		copyRegion.dstOffset = dst_offset;

		commandBuffer.copyBuffer( src, dst, 1, &copyRegion );

		endSingleTimeCommands( commandBuffer );
	}

	vk::Result Device::setDebugUtilsObjectName( [[maybe_unused]] const vk::DebugUtilsObjectNameInfoEXT& nameInfo )
	{
#ifndef NDEBUG
		if ( device().setDebugUtilsObjectNameEXT( &nameInfo ) != vk::Result::eSuccess )
		{
			std::cout << "Failed to set debug object name" << std::endl;
			throw std::runtime_error( "Failed to set debug object name" );
		}
#endif
		return vk::Result::eSuccess;
	}

} // namespace fgl::engine