#include "Device.hpp"

#include "engine/descriptors/DescriptorPool.hpp"

// std headers
#include <tracy/Tracy.hpp>

#include <cstring>
#include <iostream>
#include <set>

#include "debug/Track.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine
{

	Device* global_device { nullptr };

	Device& Device::getInstance()
	{
		assert( global_device && "Device not initialized" );
		return *global_device;
	}

	vk::PhysicalDeviceFeatures Device::DeviceCreateInfo::getDeviceFeatures( PhysicalDevice& physical_device )
	{
		const vk::PhysicalDeviceFeatures available_features { physical_device->getFeatures() };

		if ( available_features.samplerAnisotropy != VK_TRUE )
		{
			throw std::runtime_error( "samplerAnsitrophy not supported by device" );
		}

		if ( available_features.multiDrawIndirect != VK_TRUE )
		{
			throw std::runtime_error( "multiDrawIndirect not supported by device" );
		}

		if ( available_features.tessellationShader != VK_TRUE )
		{
			throw std::runtime_error( "Tesselation shader not supported by device" );
		}

		if ( available_features.drawIndirectFirstInstance != VK_TRUE )
		{
			throw std::runtime_error( "drawIndirectFirstInstance not supported by device" );
		}

		if ( available_features.wideLines != VK_TRUE )
		{
			throw std::runtime_error( "wideLines not supported by device" );
		}

#ifndef NDEBUG
		if ( available_features.robustBufferAccess != VK_TRUE )
		{
			throw std::runtime_error( "robustBufferAccess not supported by device" );
		}
#endif

		//Set enabled features
		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.multiDrawIndirect = VK_TRUE;
		deviceFeatures.tessellationShader = VK_TRUE;
		deviceFeatures.drawIndirectFirstInstance = VK_TRUE;
		deviceFeatures.wideLines = VK_TRUE;
#ifndef NDEBUG
		deviceFeatures.robustBufferAccess = VK_TRUE;
#endif

		return deviceFeatures;
	}

	void Device::DeviceCreateInfo::getIndexingFeatures()
	{
		m_indexing_features.setRuntimeDescriptorArray( VK_TRUE );
		m_indexing_features.setDescriptorBindingPartiallyBound( VK_TRUE );
		m_indexing_features.setShaderSampledImageArrayNonUniformIndexing( VK_TRUE );
		m_indexing_features.setDescriptorBindingSampledImageUpdateAfterBind( VK_TRUE );
		m_indexing_features.setDescriptorBindingUniformBufferUpdateAfterBind( VK_TRUE );
	}

	void Device::DeviceCreateInfo::getDynamicRenderingFeatures()
	{
		m_dynamic_rendering_features.setDynamicRendering( VK_TRUE );
		m_dynamic_rendering_local_read_features.setDynamicRenderingLocalRead( VK_TRUE );
		m_dynamic_rendering_unused_features.setDynamicRenderingUnusedAttachments( VK_TRUE );
		m_info_chain.unlink< vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT >();
	}

	std::vector< vk::DeviceQueueCreateInfo > Device::DeviceCreateInfo::
		getQueueCreateInfos( PhysicalDevice& physical_device )
	{
		std::vector< vk::DeviceQueueCreateInfo > queueCreateInfos;
		std::set< std::uint32_t > uniqueQueueFamilies = {
			physical_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics ),
			physical_device.queueInfo().getPresentIndex(),
		};

		//TODO: Store this somewhere where it doesn't need to be static
		static float queuePriority = 1.0f;
		for ( uint32_t queueFamily : uniqueQueueFamilies )
		{
			vk::DeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back( queueCreateInfo );
		}

		return queueCreateInfos;
	}

	void Device::DeviceCreateInfo::getCreateInfo( PhysicalDevice& physical_device )
	{
		m_create_info.setQueueCreateInfos( m_queue_create_infos );

		m_create_info.setPEnabledFeatures( &m_requested_features );

		m_create_info.setPEnabledExtensionNames( m_device_extensions );

		//Get device extension list
		const auto supported_extensions { physical_device.handle().enumerateDeviceExtensionProperties() };
		std::cout << "Supported device extensions:" << std::endl;
		for ( auto& desired_ext : m_device_extensions )
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
		if ( true )
		{
			m_create_info.enabledLayerCount = static_cast< uint32_t >( m_validation_layers.size() );
			m_create_info.ppEnabledLayerNames = m_validation_layers.data();
		}
		else
		{
			m_create_info.enabledLayerCount = 0;
		}
	}

	Device::DeviceCreateInfo::DeviceCreateInfo( PhysicalDevice& physical_device ) :
	  m_requested_features( getDeviceFeatures( physical_device ) ),
	  m_queue_create_infos( getQueueCreateInfos( physical_device ) )
	{
		getIndexingFeatures();
		getDynamicRenderingFeatures();
		getCreateInfo( physical_device );
	}

	vk::CommandPoolCreateInfo Device::commandPoolInfo()
	{
		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = m_physical_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics );
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient;

		return poolInfo;
	}

	// class member functions
	Device::Device( Window& window, Instance& instance ) :
	  m_instance( instance ),
	  m_surface_khr( window, instance ),
	  m_physical_device( m_instance, m_surface_khr ),
	  device_creation_info( m_physical_device ),
	  m_device( m_physical_device, device_creation_info.m_create_info ),
	  m_command_pool( createGraphicsPool( m_device, m_physical_device ) ),
	  m_commandPool( m_device.createCommandPool( commandPoolInfo() ) ),
	  m_graphics_queue( m_device
	                        .getQueue( m_physical_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics ), 0 ) ),
	  m_present_queue( m_device.getQueue( m_physical_device.queueInfo().getPresentIndex(), 0 ) ),
	  m_allocator( createVMAAllocator() ),
	  m_properties( m_physical_device.handle().getProperties() )
	{
		assert( !global_device );

		assert( *m_graphics_queue );
		assert( *m_present_queue );

		global_device = this;

		DescriptorPool::init();
	}

	Device::~Device()
	{
		vmaDestroyAllocator( m_allocator );

		const auto leftover_tracks { debug::getAllTracks() };

		for ( auto& track : leftover_tracks )
		{
			log::critical( "Important allocation leftover from {}:{}\n{}", track.group, track.name, track.trace );
		}
	}

	bool Device::checkValidationLayerSupport()
	{
		std::vector< vk::LayerProperties > availableLayers { vk::enumerateInstanceLayerProperties() };

		for ( const char* layerName : m_validation_layers )
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

	bool Device::checkDeviceExtensionSupport( vk::raii::PhysicalDevice device )
	{
		const std::vector< vk::ExtensionProperties > availableExtensions {
			device.enumerateDeviceExtensionProperties()
		};

		std::set< std::string > requiredExtensions( m_device_extensions.begin(), m_device_extensions.end() );

		std::uint32_t required_count { static_cast< std::uint32_t >( requiredExtensions.size() ) };
		std::uint32_t found_count { 0 };

		for ( const auto required : m_device_extensions )
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

	SwapChainSupportDetails Device::querySwapChainSupport( vk::raii::PhysicalDevice device )
	{
		SwapChainSupportDetails details;

		details.capabilities = device.getSurfaceCapabilitiesKHR( m_surface_khr );
		details.formats = device.getSurfaceFormatsKHR( m_surface_khr );
		details.presentModes = device.getSurfacePresentModesKHR( m_surface_khr );

		return details;
	}

	vk::Format Device::findSupportedFormat(
		const std::vector< vk::Format >& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features )
	{
		for ( vk::Format format : candidates )
		{
			vk::FormatProperties props { m_physical_device.handle().getFormatProperties( format ) };

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
		vk::PhysicalDeviceMemoryProperties memProperties { m_physical_device.handle().getMemoryProperties() };
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

	vk::raii::CommandBuffer Device::beginSingleTimeCommands()
	{
		ZoneScoped;
		vk::CommandBufferAllocateInfo allocInfo {};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		auto command_buffers { m_device.allocateCommandBuffers( allocInfo ) };

		assert( command_buffers.size() == 1 );

		vk::raii::CommandBuffer command_buffer { std::move( command_buffers[ 0 ] ) };

		// vk::CommandBufferBeginInfo beginInfo {};
		// beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		// command_buffer.begin( beginInfo );

		return command_buffer;
	}

	void Device::endSingleTimeCommands( vk::raii::CommandBuffer& commandBuffer )
	{
		ZoneScoped;
		// commandBuffer.end();

		vk::SubmitInfo submitInfo {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &( *commandBuffer );

		std::vector< vk::SubmitInfo > submit_infos { submitInfo };

		m_graphics_queue.submit( submit_infos );

		m_graphics_queue.waitIdle();

		//m_device.freeCommandBuffers( m_commandPool, 1, &commandBuffer );
	}

	void Device::
		copyBufferToImage( vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount )
	{
		ZoneScoped;
		vk::raii::CommandBuffer commandBuffer { beginSingleTimeCommands() };

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

		std::vector< vk::BufferImageCopy > regions { region };

		commandBuffer.copyBufferToImage( buffer, image, vk::ImageLayout::eTransferDstOptimal, regions );

		endSingleTimeCommands( commandBuffer );
	}

	VmaAllocator Device::createVMAAllocator()
	{
		VmaVulkanFunctions vk_func {};
		vk_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vk_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo create_info {};
		create_info.physicalDevice = *m_physical_device.handle();
		create_info.device = *m_device;
		create_info.pVulkanFunctions = &vk_func;
		create_info.instance = m_instance;
		create_info.vulkanApiVersion = VK_API_VERSION_1_0;

		VmaAllocator allocator;

		if ( vmaCreateAllocator( &create_info, &allocator ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create VMA allocator" );

		return allocator;
	}

	vk::Result Device::setDebugUtilsObjectName( const vk::DebugUtilsObjectNameInfoEXT& nameInfo )
	{
#ifndef NDEBUG
		device().setDebugUtilsObjectNameEXT( nameInfo );
#endif
		return vk::Result::eSuccess;
	}

} // namespace fgl::engine