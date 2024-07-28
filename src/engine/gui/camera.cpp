//
// Created by kj16609 on 7/23/24.
//

#include "engine/camera/Camera.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include <imgui.h>
#pragma GCC diagnostic pop

#include "engine/FrameInfo.hpp"
#include "preview.hpp"

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

			const auto name { std::format( "Camera: {}", camera.getIDX() ) };
			ImGui::Begin( name.c_str() );

			drawRenderingOutputs( info, camera );

			ImGui::End();
		}
	}

} // namespace fgl::engine::gui