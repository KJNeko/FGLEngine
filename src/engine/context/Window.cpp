//
// Created by kj16609 on 6/1/25.
//
#include "Window.hpp"

#include "EngineContext.hpp"
#include "Instance.hpp"
#include "catch2/catch_test_macros.hpp"

namespace fgl::engine
{
	GLFWwindow* initGlfwWindow( const vk::Extent2D extent, const std::string& name )
	{
		glfwInit();
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );

		GLFWmonitor* monitor { nullptr };
		GLFWwindow* share { nullptr };
		return glfwCreateWindow(
			static_cast< int >( extent.width ), static_cast< int >( extent.height ), name.data(), monitor, share );
	}

	void GLFWWindow::framebufferResizeCallback( GLFWwindow* window, int width, int height )
	{
		auto* ctx { static_cast< GLFWWindow* >( glfwGetWindowUserPointer( window ) ) };
		ctx->m_framebuffer_resized = true;
		ctx->m_extent = vk::Extent2D { static_cast< unsigned int >( width ), static_cast< unsigned int >( height ) };
	}

	GLFWWindow::GLFWWindow( const vk::Extent2D extent, const std::string& window_name ) :
	  m_extent( extent ),
	  m_name( window_name ),
	  m_window( initGlfwWindow( m_extent, m_name ) )
	{
		if ( !m_window ) throw std::runtime_error( "Failed to open glfw window" );

		glfwSetWindowUserPointer( m_window, this );
		glfwSetFramebufferSizeCallback( m_window, framebufferResizeCallback );
	}

	GLFWWindow::~GLFWWindow()
	{
		if ( m_window ) glfwDestroyWindow( m_window );
		glfwTerminate();
	}

	VkSurfaceKHR GLFWWindow::createSurface( const Instance& instance ) const
	{
		VkSurfaceKHR surface { VK_NULL_HANDLE };
		if ( glfwCreateWindowSurface( instance, m_window, nullptr, &surface ) != VK_SUCCESS )
		{
			const char* error = nullptr;
			glfwGetError( &error );
			throw std::runtime_error(
				std::string( "Failed to create window surface: " ) + ( error ? error : "Unknown error" ) );
		}

		return surface;
	}

} // namespace fgl::engine
