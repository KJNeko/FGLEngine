//
// Created by kj16609 on 11/27/23.
//

#include "Window.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <stdexcept>

#include "engine/Device.hpp"

namespace fgl::engine
{
	Window::Window( const int w, const int h, std::string window_name ) :
	  m_width( w ),
	  m_height( h ),
	  m_name( window_name )
	{
		initWindow();
		Device::init( *this );
	}

	Window::~Window()
	{
		glfwDestroyWindow( m_window );
		glfwTerminate();
	}

	void Window::initWindow()
	{
		glfwInit();
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

		m_window = glfwCreateWindow( m_width, m_height, m_name.data(), nullptr, nullptr );
		//ImGui_ImplGlfw_InitForVulkan( m_window, true );
		glfwSetWindowUserPointer( m_window, this );
		glfwSetFramebufferSizeCallback( m_window, framebufferResizeCallback );
	}

	vk::SurfaceKHR Window::createWindowSurface( vk::Instance instance )
	{
		VkSurfaceKHR temp_surface { VK_NULL_HANDLE };
		if ( glfwCreateWindowSurface( instance, m_window, nullptr, &temp_surface ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create window surface" );

		return vk::SurfaceKHR( temp_surface );
	}

	void Window::framebufferResizeCallback( GLFWwindow* glfw_window, int width, int height )
	{
		auto window { reinterpret_cast< Window* >( glfwGetWindowUserPointer( glfw_window ) ) };
		window->frame_buffer_resized = true;
		window->m_width = width;
		window->m_height = height;
	}

} // namespace fgl::engine