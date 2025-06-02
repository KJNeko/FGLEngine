//
// Created by kj16609 on 6/2/25.
//
#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace fgl::engine
{
	class WindowSurface
	{
		vk::raii::SurfaceKHR m_surface;

	  public:

		WindowSurface() = delete;

		WindowSurface( class Instance& instance, VkSurfaceKHR surface );
		WindowSurface( class Instance& instance, const class GLFWWindow& window );
	};
} // namespace fgl::engine