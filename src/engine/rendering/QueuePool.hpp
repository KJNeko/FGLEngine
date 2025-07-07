//
// Created by kj16609 on 6/20/24.
//

#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{
	class Surface;
	class PhysicalDevice;

	class QueuePool
	{
		struct QueueInfo
		{
			vk::QueueFamilyProperties props;
			bool can_present;
			std::uint8_t allocated { 0 };
		};

		std::vector< QueueInfo > queue_info {};

	  public:

		QueuePool( const vk::raii::PhysicalDevice&, Surface& );

		FGL_DELETE_ALL_RO5( QueuePool );

		using QueueIndex = std::uint32_t;

		//! Returns a unique list of indexes with the matching flags
		QueueIndex getIndex( vk::QueueFlags flags, vk::QueueFlags anti_flags = vk::QueueFlags( 0 ) );

		std::uint32_t getPresentIndex() const;
	};

} // namespace fgl::engine
