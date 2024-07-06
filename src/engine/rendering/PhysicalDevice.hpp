//
// Created by kj16609 on 6/20/24.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "QueuePool.hpp"
#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{
	class Instance;
	class Surface;

	class PhysicalDevice
	{
		vk::raii::PhysicalDevice m_phy_device;
		QueuePool queue_pool;

		FGL_DELETE_ALL_Ro5( PhysicalDevice );

		//! Picks a device that can render to the desired output window
		vk::raii::PhysicalDevice pickPhysicalDevice( Instance& dev, Surface& surface );
		bool supportsRequiredExtensions();

	  public:

		QueuePool& queueInfo() { return queue_pool; }

		PhysicalDevice( Instance& instance, Surface& surface );

		vk::raii::PhysicalDevice& handle() { return m_phy_device; }

		operator vk::raii::PhysicalDevice() { return m_phy_device; }

		VkPhysicalDevice operator*() const { return *m_phy_device; }

		vk::raii::PhysicalDevice* operator->() { return &m_phy_device; }
	};

} // namespace fgl::engine