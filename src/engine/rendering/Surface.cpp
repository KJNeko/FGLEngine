//
// Created by kj16609 on 6/20/24.
//

#include "Surface.hpp"

#include "engine/Window.hpp"

namespace fgl::engine
{

	Surface::Surface( Window& window, Instance& instance ) : m_surface( window.createWindowSurface( instance ) )
	{}

} // namespace fgl::engine
