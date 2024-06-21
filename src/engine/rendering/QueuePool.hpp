//
// Created by kj16609 on 6/20/24.
//

#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <set>

#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{
	class Surface;
	class PhysicalDevice;

	class Queue;

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

		QueuePool( vk::raii::PhysicalDevice&, Surface& );

		FGL_DELETE_ALL_Ro5( QueuePool );

		Queue allocate();
		Queue allocateIndex( const std::uint32_t idx );

		using QueueIndex = std::uint32_t;

		//! Returns a unique list of indexes with the matching flags
		QueueIndex getIndex( const vk::QueueFlags flags );

		std::uint32_t getPresentIndex();
	};

} // namespace fgl::engine
