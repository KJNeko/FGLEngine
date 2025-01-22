//
// Created by kj16609 on 1/21/25.
//
#pragma once
#include "FileBrowser.hpp"

namespace fgl::editor
{
	class EditorGuiContext
	{
		engine::filesystem::FileBrowser m_file_browser {};

	  public:

		EditorGuiContext( const engine::Window& window );
		~EditorGuiContext();

		void beginDraw();
		void draw( engine::FrameInfo& info );
		void endDraw( vk::raii::CommandBuffer& command_buffer );
		void endDraw( engine::FrameInfo& info );
	};
} // namespace fgl::editor