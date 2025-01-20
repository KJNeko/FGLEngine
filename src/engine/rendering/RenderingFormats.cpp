//
// Created by kj16609 on 12/9/24.
//

#include "RenderingFormats.hpp"

#include "PresentSwapChain.hpp"
#include "devices/Device.hpp"

namespace fgl::engine
{

	vk::Format pickColorFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eR8G8B8A8Unorm };

		return Device::getInstance()
		    .findSupportedFormat( formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage );
	}

	vk::Format pickPositionFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eR16G16B16A16Sfloat };

		return Device::getInstance()
		    .findSupportedFormat( formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage );
	}

	vk::Format pickNormalFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eR16G16B16A16Sfloat };

		return Device::getInstance()
		    .findSupportedFormat( formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage );
	}

	vk::Format pickDepthFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eD32Sfloat,
			                                       vk::Format::eD32SfloatS8Uint,
			                                       vk::Format::eD24UnormS8Uint };

		return Device::getInstance().findSupportedFormat(
			formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment );
	}

	vk::Format pickMetallicFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eR16G16B16A16Sfloat };

		return Device::getInstance()
		    .findSupportedFormat( formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage );
	}

	vk::Format pickEmissiveFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eR16G16B16A16Sfloat };

		return Device::getInstance()
		    .findSupportedFormat( formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage );
	}

	vk::Format pickCompositeFormat()
	{
		static std::vector< vk::Format > formats { vk::Format::eR8G8B8A8Unorm };

		return Device::getInstance()
		    .findSupportedFormat( formats, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage );
	}

	vk::Format pickPresentFormat()
	{
		ZoneScoped;

		const auto swapchain_details { Device::getInstance().getSwapChainSupport() };
		const auto& available_formats { swapchain_details.formats };

		for ( const auto& format : available_formats )
		{
			if ( format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
			{
				return format.format;
			}
		}

		throw std::runtime_error( "failed to find a present format" );
	}

} // namespace fgl::engine
