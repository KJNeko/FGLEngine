//
// Created by kj16609 on 6/20/24.
//

#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{
	class Window;
	class Instance;

	class Surface
	{
		vk::raii::SurfaceKHR m_surface;

	  public:

		Surface( Window& window, Instance& instance );

		FGL_DELETE_ALL_RO5( Surface );

		vk::raii::SurfaceKHR& handle() { return m_surface; }

		operator vk::SurfaceKHR() { return m_surface; }

		operator VkSurfaceKHR() const { return *m_surface; }
	};
} // namespace fgl::engine