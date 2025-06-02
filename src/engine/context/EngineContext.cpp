//
// Created by kj16609 on 6/1/25.
//

#include "EngineContext.hpp"

#include "Instance.hpp"
#include "Window.hpp"
#include "WindowSurface.hpp"

namespace fgl::engine
{

	EngineContext::EngineContext() :
	  m_window( std::make_unique< GLFWWindow >( vk::Extent2D { 1280, 720 }, "Engine" ) ),
	  m_instance( std::make_unique< Instance >( m_vk_ctx ) ),
	  m_surface( std::make_unique< WindowSurface >( *m_instance, *m_window ) )
	{}

	EngineContext::~EngineContext()
	{}

} // namespace fgl::engine
