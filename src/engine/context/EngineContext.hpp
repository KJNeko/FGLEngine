//
// Created by kj16609 on 6/1/25.
//
#pragma once

#include <vulkan/vulkan_raii.hpp>

#ifndef TEST_PUBLIC
#define TEST_PUBLIC
#endif

namespace fgl::engine
{
	class EngineContext
	{
		TEST_PUBLIC
		vk::raii::Context m_vk_ctx {};

		std::unique_ptr< class GLFWWindow > m_window;
		std::unique_ptr< class Instance > m_instance;
		std::unique_ptr< class WindowSurface > m_surface;

	  public:

		EngineContext();
		~EngineContext();
	};

} // namespace fgl::engine
