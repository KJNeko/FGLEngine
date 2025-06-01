//
// Created by kj16609 on 6/3/24.
//

#pragma once

namespace fgl::engine
{
	namespace memory
	{
		class BufferHandle;
	}
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

	void drawBufferInfo( const memory::BufferHandle& buffer );

	void drawObject( GameObject& game_object );
	void drawComponentsList( GameObject& game_object );
	void drawSelectedComponent();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
	void drawTransferManager();
#pragma GCC diagnostic pop

	void drawCameraOutputs( FrameInfo& info );

	void drawStats( const FrameInfo& info );

} // namespace fgl::engine::gui
