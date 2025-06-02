//
// Created by kj16609 on 6/1/25.
//
#pragma once

#define GLFW_INCLUDE_VULKAN
// TODO: Figure out alternative windows
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

#include "EngineContext.hpp"

#ifndef TEST_PUBLIC
#define TEST_PUBLIC
#endif

namespace fgl::engine
{
	class GLFWWindow
	{
		TEST_PUBLIC
		vk::Extent2D m_extent {};
		bool m_framebuffer_resized { false };

		std::string m_name;

		::GLFWwindow* m_window;

		static void framebufferResizeCallback( GLFWwindow* window, int width, int height );

	  public:

		GLFWWindow() = delete;
		GLFWWindow( vk::Extent2D extent, const std::string& window_name );
		~GLFWWindow();

		VkSurfaceKHR createSurface( const Instance& instance ) const;

		vk::Extent2D getExtent() const { return m_extent; }
	};

} // namespace fgl::engine