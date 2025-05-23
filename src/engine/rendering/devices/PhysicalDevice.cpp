//
// Created by kj16609 on 6/20/24.
//

#include "PhysicalDevice.hpp"

#include <algorithm>

#include "engine/debug/logging/logging.hpp"
#include "engine/rendering/Instance.hpp"
#include "engine/rendering/Surface.hpp"

namespace fgl::engine
{

	PhysicalDevice::PhysicalDevice( Instance& instance, Surface& surface ) :
	  m_phy_device( pickPhysicalDevice( instance, surface ) ),
	  queue_pool( m_phy_device, surface )
	{}

	//! Checks that a device has a graphics capable queue and can present to the given surface.
	bool isDeviceSupported( const vk::raii::PhysicalDevice& device, Surface& surface )
	{
		const std::vector< vk::QueueFamilyProperties > family_props { device.getQueueFamilyProperties() };

		bool has_graphics_queue { false };
		bool has_present_queue { false };

		int idx { 0 };
		for ( const auto& queue_family : family_props )
		{
			if ( queue_family.queueCount <= 0 )
			{
				++idx;
				continue;
			}

			// We need at least 1 queue that can do graphics.
			if ( queue_family.queueFlags & vk::QueueFlagBits::eGraphics )
			{
				has_graphics_queue = true;
			}

			// Check if the device can present to the surface.
			vk::Bool32 can_present { device.getSurfaceSupportKHR( idx, surface ) };
			if ( can_present == VK_TRUE )
			{
				has_present_queue = true;
			}

			if ( has_graphics_queue && has_present_queue ) return true;

			++idx;
		}

		return false;
	}

	float scoreDevice( const vk::raii::PhysicalDevice& device )
	{
		constexpr float api_discrete_factor { 100.0f };
		constexpr float api_major_factor { 1.0f };
		constexpr float api_minor_factor { 1.0f };

		const auto props { device.getProperties() };

		float score { 1.0f };
		score += static_cast< float >( props.apiVersion );
		if ( props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ) score *= api_discrete_factor;

		return score;
	}

	void rankDevices( std::vector< vk::raii::PhysicalDevice >& vector )
	{
		std::ranges::sort(
			vector,
			[]( const vk::raii::PhysicalDevice& a, const vk::raii::PhysicalDevice& b )
			{
				const auto a_score { scoreDevice( a ) };
				const auto b_score { scoreDevice( b ) };

				return a_score > b_score;
			} );
	}

	vk::raii::PhysicalDevice PhysicalDevice::pickPhysicalDevice( Instance& instance, Surface& surface )
	{
		std::vector< vk::raii::PhysicalDevice > devices { instance.handle().enumeratePhysicalDevices() };

		log::debug( "Can select {} devices", devices.size() );
		rankDevices( devices );

		for ( auto& device : devices )
		{
			if ( isDeviceSupported( device, surface ) )
			{
				//We found a device we can use.
				log::info( "Found device for surface" );

				vk::raii::PhysicalDevice selected_device { std::move( device ) };

				return selected_device;
			}
			log::debug( "Skipping devices as it is not supported" );
		}

		throw std::runtime_error( "Failed to find a valid device" );
	}

	const static std::vector< const char* > required_device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		                                                                 VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
#if ENABLE_CALIBRATED_PROFILING
		                                                                 VK_KHR_CALIBRATED_TIMESTAMPS_EXTENSION_NAME
#endif
	};

	bool PhysicalDevice::supportsRequiredExtensions()
	{
		const std::vector< vk::ExtensionProperties > device_extentions {
			m_phy_device.enumerateDeviceExtensionProperties()
		};

		for ( const auto required : required_device_extensions )
		{
			if ( std::find_if(
					 device_extentions.begin(),
					 device_extentions.end(),
					 [ &required ]( const vk::ExtensionProperties& props )
					 { return std::strcmp( props.extensionName, required ); } )
			     == device_extentions.end() )
			{
				return false;
			}
		}

		return true;
	}

} // namespace fgl::engine
