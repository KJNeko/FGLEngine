//
// Created by kj16609 on 11/28/23.
//

#include "KeyboardMovementController.hpp"

#include <bits/chrono.h>

#include <iostream>

#include "engine/primitives/vectors/Vector.hpp"
#include "gameobjects/GameObject.hpp"

namespace fgl::engine
{

	void setCursorPos( GLFWwindow* window, const glm::vec2 pos )
	{
		glfwSetCursorPos( window, static_cast< double >( pos.x ), static_cast< double >( pos.y ) );
	}

	glm::vec2 getCursorPos( GLFWwindow* window )
	{
		//Take cursor input
		double xpos { 0.0 };
		double ypos { 0.0 };
		glfwGetCursorPos( window, &xpos, &ypos );
		return { xpos, ypos };
	}

	void KeyboardMovementController::moveInPlaneXZ( GLFWwindow* window, float dt, fgl::engine::GameObject& target )
	{
		assert( window );
		constexpr float rotation_rate { 1.0f };
		constexpr float pitch_modifier { 1.0f };
		constexpr float yaw_modifier { 1.0f };

		float pitch_change { 0.0f };
		float yaw_change { 0.0f };

		if ( glfwGetKey( window, key_mappings.look_right ) == GLFW_PRESS ) yaw_change += rotation_rate * yaw_modifier;
		if ( glfwGetKey( window, key_mappings.look_left ) == GLFW_PRESS ) yaw_change -= rotation_rate * yaw_modifier;
		if ( glfwGetKey( window, key_mappings.look_up ) == GLFW_PRESS ) pitch_change += rotation_rate * pitch_modifier;
		if ( glfwGetKey( window, key_mappings.look_down ) == GLFW_PRESS )
			pitch_change -= rotation_rate * pitch_modifier;

		static bool cursor_enabled { true };
		static bool cursor_restored { true };
		static glm::vec2 stored_cursor_pos {};
		static std::chrono::time_point last_pressed { std::chrono::steady_clock::now() };

		if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
		{
			const auto now { std::chrono::steady_clock::now() };
			using namespace std::chrono_literals;
			if ( now - last_pressed > 500ms )
			{
				cursor_enabled = !cursor_enabled;
				last_pressed = now;
			}
		}

		if ( cursor_enabled && !cursor_restored )
		{
			glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			setCursorPos( window, stored_cursor_pos );
			std::cout << "Restored cursor pos: " << stored_cursor_pos.x << "x" << stored_cursor_pos.y << std::endl;
			cursor_restored = true;
		}
		else if ( !cursor_enabled && cursor_restored )
		{
			stored_cursor_pos = getCursorPos( window );
			std::cout << "Stored cursor pos: " << stored_cursor_pos.x << "x" << stored_cursor_pos.y << std::endl;
			cursor_restored = false;
			glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
			setCursorPos( window, { 0, 0 } );
		}

		if ( cursor_enabled )
		{
			const auto& original_rotation { target.getTransform().rotation };
			Rotation yaw_rotation {};
			Rotation pitch_rotation {};

			if ( pitch_change > std::numeric_limits< float >::epsilon()
			     || pitch_change < std::numeric_limits< float >::epsilon() )
				pitch_rotation.pitch() += ( dt * pitch_change );

			if ( yaw_change > std::numeric_limits< float >::epsilon()
			     || yaw_change < std::numeric_limits< float >::epsilon() )
				yaw_rotation.yaw() += ( dt * yaw_change );

			target.getTransform().rotation = yaw_rotation * original_rotation * pitch_rotation;
		}
		else // No cursor
		{
			const auto pos { getCursorPos( window ) };
			const float xpos { pos.x };
			const float ypos { pos.y };

			Rotation target_rotation { target.getTransform().rotation };

			target_rotation.yaw() += ( xpos * 0.006f ) * look_speed;
			target_rotation.pitch() -= ( ypos * 0.006f ) * look_speed;

			target.getTransform().rotation = target_rotation;

			setCursorPos( window, { 0, 0 } );
		}

		const Vector forward_dir { target.getTransform().rotation.forward() };
		const Vector up_dir { target.getTransform().rotation.up() };
		const Vector right_dir { target.getTransform().rotation.right() };

		Vector move_dir { 0.0f };
		if ( glfwGetKey( window, key_mappings.move_forward ) == GLFW_PRESS ) move_dir += forward_dir;
		if ( glfwGetKey( window, key_mappings.move_backward ) == GLFW_PRESS ) move_dir -= forward_dir;
		if ( glfwGetKey( window, key_mappings.move_right ) == GLFW_PRESS ) move_dir += right_dir;
		if ( glfwGetKey( window, key_mappings.move_left ) == GLFW_PRESS ) move_dir -= right_dir;
		if ( glfwGetKey( window, key_mappings.move_up ) == GLFW_PRESS ) move_dir += up_dir;
		if ( glfwGetKey( window, key_mappings.move_down ) == GLFW_PRESS ) move_dir -= up_dir;

		const NormalVector n_move_dir { move_dir };

		if ( glm::dot( move_dir.vec(), move_dir.vec() ) > std::numeric_limits< float >::epsilon() )
			target.getTransform().translation += n_move_dir * ( move_speed * dt );
	}

} // namespace fgl::engine