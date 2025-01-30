//
// Created by kj16609 on 6/3/24.
//

#pragma once

namespace fgl::engine
{
	class GameObject;
	class Device;
	class Renderer;
	class Window;
	struct FrameInfo;
} // namespace fgl::engine

namespace fgl::engine::gui
{
	// Setup/Destruction
	void cleanupImGui();

	void drawDock();

	void drawMenubar( FrameInfo& info );

	void drawImGui( FrameInfo& );
	void drawEntityGUI( FrameInfo& );

	void drawEntityInfo( FrameInfo& );

	void drawObject( GameObject& game_object );
	void drawComponentsList( GameObject& game_object );
	void drawSelectedComponent();

	void drawCameraOutputs( FrameInfo& info );

	void drawStats( const FrameInfo& info );

} // namespace fgl::engine::gui
