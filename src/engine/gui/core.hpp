//
// Created by kj16609 on 6/3/24.
//

#pragma once
#include "engine/rendering/Renderer.hpp"

namespace fgl::engine
{
	class Device;
	class Renderer;
	class Window;
} // namespace fgl::engine

namespace fgl::engine::gui
{
	void initGui( const Window& window, const Renderer& renderer );
	void cleanupImGui();

	void drawMainGUI( FrameInfo& );
	void drawEntityGUI( FrameInfo& );

} // namespace fgl::engine::gui
