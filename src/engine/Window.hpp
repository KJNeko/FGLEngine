//
// Created by kj16609 on 11/27/23.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

#include "rendering/Surface.hpp"

namespace fgl::engine
{

	class Window
	{
		GLFWwindow* m_window { nullptr };

		int m_width;
		int m_height;
		bool frame_buffer_resized { false };

		std::string m_name;

		void initWindow();

		static void framebufferResizeCallback( GLFWwindow* window, int width, int height );

	  public:

		bool wasWindowResized() { return frame_buffer_resized; }

		void resetWindowResizedFlag() { frame_buffer_resized = false; }

		bool shouldClose() { return glfwWindowShouldClose( m_window ); }

		vk::raii::SurfaceKHR createWindowSurface( Instance& instance );

		VkExtent2D getExtent()
		{
			return { static_cast< std::uint32_t >( m_width ), static_cast< std::uint32_t >( m_height ) };
		}

		GLFWwindow* window() const { return m_window; }

		Window( const int w, const int h, std::string window_name );
		Window() = delete;
		Window( const Window& other ) = delete;
		Window( Window&& other ) = delete;
		Window& operator=( Window& other ) = delete;
		~Window();
	};
} // namespace fgl::engine