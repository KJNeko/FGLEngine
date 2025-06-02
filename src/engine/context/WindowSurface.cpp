//
// Created by kj16609 on 6/2/25.
//
#include "WindowSurface.hpp"

#include "Instance.hpp"
#include "Window.hpp"

namespace fgl::engine
{

	WindowSurface::WindowSurface( Instance& instance, const VkSurfaceKHR surface ) :
	  m_surface( instance.vkHandle(), surface )
	{}

	WindowSurface::WindowSurface( Instance& instance, const GLFWWindow& window ) :
	  m_surface( instance.vkHandle(), window.createSurface( instance ) )
	{}
} // namespace fgl::engine