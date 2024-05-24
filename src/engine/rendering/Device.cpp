#include "Device.hpp"

#include "engine/descriptors/DescriptorPool.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

#include "engine/logging.hpp"

PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT { nullptr };
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT { nullptr };
PFN_vkSetDebugUtilsObjectNameEXT pfnVkSetDebugUtilsObjectNameEXT { nullptr };

// local callback functions
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData )
{
	using Bits = VkDebugUtilsMessageSeverityFlagBitsEXT;

	if ( pCallbackData->flags & Bits::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
	{
		spdlog::info( pCallbackData->pMessage );
	}
	if ( pCallbackData->flags & Bits::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
	{
		spdlog::warn( pCallbackData->pMessage );
	}
	if ( pCallbackData->flags & Bits::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
	{
		spdlog::error( pCallbackData->pMessage );
		std::abort();
	}

	if ( pCallbackData->flags
	     & ( VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
	         | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) )
		throw std::runtime_error( pCallbackData->pMessage );
	else
		std::cout << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* create_info,
	const VkAllocationCallbacks* allocator,
	VkDebugUtilsMessengerEXT* messenger )
{
	return pfnVkCreateDebugUtilsMessengerEXT( instance, create_info, allocator, messenger );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const * pAllocator )
{
	return pfnVkDestroyDebugUtilsMessengerEXT( instance, messenger, pAllocator );
}

VKAPI_ATTR VkResult VKAPI_CALL
	vkSetDebugUtilsObjectNameEXT( VkDevice device, const VkDebugUtilsObjectNameInfoEXT* nameInfo )
{
	return pfnVkSetDebugUtilsObjectNameEXT( device, nameInfo );
}

namespace fgl::engine
{
	vk::Result CreateDebugUtilsMessengerEXT(
		vk::Instance instance,
		const vk::DebugUtilsMessengerCreateInfoEXT& pCreateInfo,
		[[maybe_unused]] const vk::AllocationCallbacks* pAllocator,
		vk::DebugUtilsMessengerEXT* pDebugMessenger )
	{
		return instance.createDebugUtilsMessengerEXT( &pCreateInfo, pAllocator, pDebugMessenger );
	}

	void DestroyDebugUtilsMessengerEXT(
		vk::Instance instance,
		vk::DebugUtilsMessengerEXT debugMessenger,
		[[maybe_unused]] const vk::AllocationCallbacks* pAllocator )
	{
		instance.destroyDebugUtilsMessengerEXT( debugMessenger );
	}

	inline static std::unique_ptr< Device > global_device { nullptr };

	Device& Device::init( Window& window )
	{
		global_device = std::make_unique< Device >( window );
		DescriptorPool::init( *global_device );
		return *global_device;
	}

	Device& Device::getInstance()
	{
		assert( global_device && "Device not initialized" );
		return *global_device;
	}

	// class member functions
	Device::Device( Window& window ) : m_window { window }
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createVMAAllocator();
		createCommandPool();
		initImGui();
	}

	void Device::initImGui()
	{}

	Device::~Device()
	{
		vkDestroyCommandPool( device_, m_commandPool, nullptr );
		vkDestroyDevice( device_, nullptr );

		if ( enableValidationLayers )
		{
			DestroyDebugUtilsMessengerEXT( m_instance, m_debugMessenger, nullptr );
		}

		vkDestroySurfaceKHR( m_instance, surface_, nullptr );
		vkDestroyInstance( m_instance, nullptr );
	}

	void Device::createInstance()
	{
		if ( enableValidationLayers && !checkValidationLayerSupport() )
		{
			throw std::runtime_error( "validation layers requested, but not available!" );
		}

		vk::ApplicationInfo appInfo {};
		appInfo.pApplicationName = "Mecha Game";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.pEngineName = "titor";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_3;

		vk::InstanceCreateInfo createInfo {};
		createInfo.pApplicationInfo = &appInfo;

		auto extensions { getRequiredInstanceExtensions() };
		assert( extensions.size() >= 1 );
		createInfo.enabledExtensionCount = static_cast< uint32_t >( extensions.size() );
		createInfo.ppEnabledExtensionNames = extensions.data();

		hasGlfwRequiredInstanceExtensions();

		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
		if ( enableValidationLayers )
		{
			createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo( debugCreateInfo );
			createInfo.pNext = &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if ( vk::createInstance( &createInfo, nullptr, &m_instance ) != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to create Vulkan instance" );
	}

	void Device::pickPhysicalDevice()
	{
		std::vector< vk::PhysicalDevice > devices { m_instance.enumeratePhysicalDevices() };

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

		if ( auto res = m_physical_device.createDevice( &createInfo, nullptr, &device_ ); res != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create logical device!" );
		}

		device_.getQueue( indices.graphicsFamily, 0, &graphicsQueue_ );
		device_.getQueue( indices.presentFamily, 0, &presentQueue_ );
	}

	void Device::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient;

		if ( device_.createCommandPool( &poolInfo, nullptr, &m_commandPool ) != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to create command pool!" );
		}
	}

	void Device::createSurface()
	{
		surface_ = m_window.createWindowSurface( m_instance );
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

	void Device::populateDebugMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& createInfo )
	{
		createInfo.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
		                       | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		                       | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	void Device::setupDebugMessenger()
	{
		std::cout << "Setting up debug messenger: " << std::endl;

		if ( !enableValidationLayers )
		{
			std::cout << "-- Validation disabled" << std::endl;
			return;
		}

		pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<
			PFN_vkCreateDebugUtilsMessengerEXT >( m_instance.getProcAddr( "vkCreateDebugUtilsMessengerEXT" ) );
		pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<
			PFN_vkDestroyDebugUtilsMessengerEXT >( m_instance.getProcAddr( "vkDestroyDebugUtilsMessengerEXT" ) );
		pfnVkSetDebugUtilsObjectNameEXT =
			reinterpret_cast< PFN_vkSetDebugUtilsObjectNameEXT >( m_instance
		                                                              .getProcAddr( "vkSetDebugUtilsObjectNameEXT" ) );

		if ( !pfnVkCreateDebugUtilsMessengerEXT || !pfnVkDestroyDebugUtilsMessengerEXT )
		{
			throw std::runtime_error( "failed to load debug messenger functions!" );
		}

		if ( !pfnVkSetDebugUtilsObjectNameEXT )
		{
			throw std::runtime_error( "failed to load debug object name function!" );
		}

		vk::DebugUtilsMessengerCreateInfoEXT createInfo {};
		populateDebugMessengerCreateInfo( createInfo );

		if ( CreateDebugUtilsMessengerEXT( m_instance, createInfo, nullptr, &m_debugMessenger )
		     != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to set up debug messenger!" );
		}

		std::cout << "-- Debug callback setup" << std::endl;
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

	void Device::hasGlfwRequiredInstanceExtensions()
	{
		std::vector< vk::ExtensionProperties > instance_extensions { vk::enumerateInstanceExtensionProperties() };

		std::cout << "available instance instance_extensions:" << std::endl;
		std::unordered_set< std::string > available;
		for ( const auto& extension : instance_extensions )
		{
			std::cout << "\t" << extension.extensionName << std::endl;
			available.insert( extension.extensionName );
		}

		std::cout << "required instance instance_extensions:" << std::endl;
		auto requiredExtensions { getRequiredInstanceExtensions() };
		for ( const char* required : requiredExtensions )
		{
			if ( std::find_if(
					 instance_extensions.begin(),
					 instance_extensions.end(),
					 [ required ]( const vk::ExtensionProperties& prop )
					 { return std::strcmp( prop.extensionName, required ); } )
			     == instance_extensions.end() )
				throw std::runtime_error( "Missing required glfw extension" );
			else
				std::cout << required << std::endl;
		}
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
			vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface_, &presentSupport );
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

		if ( device.getSurfaceCapabilitiesKHR( surface_, &details.capabilities ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to get surface capabilities" );

		uint32_t formatCount { 0 };
		if ( device.getSurfaceFormatsKHR( surface_, &formatCount, nullptr ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to get surface formats" );

		if ( formatCount != 0 )
		{
			details.formats.resize( formatCount );
			if ( device.getSurfaceFormatsKHR( surface_, &formatCount, details.formats.data() ) != vk::Result::eSuccess )
				throw std::runtime_error( "failed to get surface formats" );
		}

		uint32_t presentModeCount { 0 };
		if ( device.getSurfacePresentModesKHR( surface_, &presentModeCount, nullptr ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to get surface present modes" );

		if ( presentModeCount != 0 )
		{
			details.presentModes.resize( presentModeCount );
			if ( device.getSurfacePresentModesKHR( surface_, &presentModeCount, details.presentModes.data() )
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
		vk::CommandBufferAllocateInfo allocInfo {};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer {};
		if ( device_.allocateCommandBuffers( &allocInfo, &commandBuffer ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to allocate command buffers!" );

		vk::CommandBufferBeginInfo beginInfo {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer.begin( beginInfo );

		return commandBuffer;
	}

	void Device::endSingleTimeCommands( vk::CommandBuffer commandBuffer )
	{
		vkEndCommandBuffer( commandBuffer );

		vk::SubmitInfo submitInfo {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if ( graphicsQueue_.submit( 1, &submitInfo, VK_NULL_HANDLE ) != vk::Result::eSuccess )
			throw std::runtime_error( "failed to submit single time command buffer!" );

		graphicsQueue_.waitIdle();

		device_.freeCommandBuffers( m_commandPool, 1, &commandBuffer );
	}

	void Device::
		copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount )
	{
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
		create_info.device = device_;
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