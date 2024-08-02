//
// Created by kj16609 on 6/3/24.
//

#pragma once

namespace fgl::engine
{
	class Device;
	class Renderer;
	class Window;
	struct FrameInfo;
} // namespace fgl::engine

namespace fgl::engine::gui
{
	void initGui( const Window& window, const Renderer& renderer );
	void cleanupImGui();

	void drawMainGUI( FrameInfo& );
	void drawEntityGUI( FrameInfo& );

	void drawEntityInfo( FrameInfo& );
	void drawFilesystemGUI( FrameInfo& info );

	void drawCameraOutputs( FrameInfo& info );

} // namespace fgl::engine::gui
