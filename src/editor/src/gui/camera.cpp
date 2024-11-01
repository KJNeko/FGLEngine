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

	void handleCameraInput( const FrameInfo& info, Camera& camera )
	{
		const auto delta_time { info.delta_time };

		//If we aren't focused. We just return.
		if ( !ImGui::IsWindowFocused() ) return;

		auto& original_rotation { camera.getTransform().rotation };

		auto& yaw_change { original_rotation };
		auto& pitch_change { original_rotation };

		constexpr double pitch_rate { 1.0 };
		constexpr double yaw_rate { 1.0 };

		if ( ImGui::IsKeyDown( ImGuiKey_UpArrow ) )
		{
			// yaw_change.xAngle() += ( delta_time * pitch_rate );
			yaw_change.addY( delta_time * pitch_rate );
		}

		if ( ImGui::IsKeyDown( ImGuiKey_DownArrow ) )
		{
			// yaw_change.xAngle() -= ( delta_time * pitch_rate );
			yaw_change.addY( -( delta_time * pitch_rate ) );
		}

		if ( ImGui::IsKeyDown( ImGuiKey_RightArrow ) )
		{
			pitch_change.addZWorld( delta_time * yaw_rate );
		}

		if ( ImGui::IsKeyDown( ImGuiKey_LeftArrow ) )
		{
			pitch_change.addZWorld( -( delta_time * yaw_rate ) );
		}

		Vector move_dir { 0.0f };

		const Vector forward { camera.getForward() };
		const Vector up { camera.getUp() };
		const Vector right { camera.getRight() };

		if ( ImGui::IsKeyDown( ImGuiKey_W ) )
		{
			move_dir += forward;
		}

		if ( ImGui::IsKeyDown( ImGuiKey_S ) )
		{
			move_dir -= forward;
		}

		if ( ImGui::IsKeyDown( ImGuiKey_D ) )
		{
			move_dir += right;
		}

		if ( ImGui::IsKeyDown( ImGuiKey_A ) )
		{
			move_dir -= right;
		}

		if ( ImGui::IsKeyDown( ImGuiKey_E ) )
		{
			move_dir += up;
		}

		if ( ImGui::IsKeyDown( ImGuiKey_Q ) )
		{
			move_dir -= up;
		}

		constexpr float move_speed { 0.5f };

		camera.getTransform().translation += move_dir * ( move_speed * delta_time );

		camera.updateMatrix();
	}

	void drawCameraOutputs( FrameInfo& info )
	{
		const auto& camera_list { info.m_camera_list };
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

			ImGui::PushStyleVar( ImGuiStyleVar_ChildBorderSize, 0.0f );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

			ImGui::Begin(
				name.c_str(),
				nullptr,
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar );

			handleCameraInput( info, camera );

			const auto [ x, y ] = ImGui::GetWindowSize();

			camera.setExtent( { static_cast< std::uint32_t >( x ), static_cast< std::uint32_t >( y ) } );

			drawRenderingOutputs( info, camera );

			ImGui::End();

			ImGui::PopStyleVar( 2 );
		}
	}

} // namespace fgl::engine::gui