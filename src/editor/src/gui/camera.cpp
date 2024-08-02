//
// Created by kj16609 on 7/23/24.
//

#include "engine/camera/Camera.hpp"

#include <cassert>

#include "engine/FrameInfo.hpp"
#include "preview.hpp"
#include "safe_include.hpp"

namespace fgl::engine::gui
{

	void drawCameraOutputs( FrameInfo& info )
	{
		auto& camera_list { info.m_camera_list };
		for ( const auto& weak_camera_ptr : camera_list )
		{
			if ( weak_camera_ptr.expired() ) continue;

			auto camera_ptr { weak_camera_ptr.lock() };

			assert( camera_ptr );

			Camera& camera { *camera_ptr };

			std::string name { "" };

			if ( camera.getName() == "" )
				name = std::format( "Camera: ID {}", camera.getIDX() );
			else
				name = camera.getName();

			ImGui::Begin(
				name.c_str(),
				nullptr,
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar );

			drawRenderingOutputs( info, camera );

			ImGui::End();
		}
	}

} // namespace fgl::engine::gui