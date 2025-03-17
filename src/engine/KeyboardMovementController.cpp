//
// Created by kj16609 on 11/28/23.
//

#include "KeyboardMovementController.hpp"

#include <bits/chrono.h>

#include <iostream>

#include "engine/primitives/vectors/Vector.hpp"
#include "gameobjects/GameObject.hpp"
#include "gameobjects/components/TransformComponent.hpp"

namespace fgl::engine
{

	void setCursorPos( GLFWwindow* window, const glm::vec2 pos )
	{
		glfwSetCursorPos( window, pos.x, pos.y );
	}

	glm::vec2 getCursorPos( GLFWwindow* window )
	{
		//Take cursor input
		double xpos { 0.0 };
		double ypos { 0.0 };
		glfwGetCursorPos( window, &xpos, &ypos );
		return { xpos, ypos };
	}

	void KeyboardMovementController::moveInPlaneXZ( GLFWwindow* window, float dt, GameObject& target )
	{
		assert( window );

		auto components { target.getComponents< components::TransformComponent >() };

		if ( components.empty() )
		{
			log::warn( "KeyboardMovementController: Attempted to move object with no transform" );
			return;
		}

		components::TransformComponent& component { *components[ 0 ] };
		WorldTransform& transform { *component };

		constexpr float rotation_rate { 1.0f };
		constexpr float pitch_modifier { 1.0f };
		constexpr float yaw_modifier { 1.0f };

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
			float pitch_change { 0.0f };
			float yaw_change { 0.0f };

			if ( glfwGetKey( window, key_mappings.look_right ) == GLFW_PRESS )
				yaw_change += rotation_rate * yaw_modifier;

			if ( glfwGetKey( window, key_mappings.look_left ) == GLFW_PRESS )
				yaw_change -= rotation_rate * yaw_modifier;

			if ( glfwGetKey( window, key_mappings.look_up ) == GLFW_PRESS )
				pitch_change += rotation_rate * pitch_modifier;

			if ( glfwGetKey( window, key_mappings.look_down ) == GLFW_PRESS )
				pitch_change -= rotation_rate * pitch_modifier;

			if ( pitch_change > std::numeric_limits< float >::epsilon()
			     || pitch_change < -std::numeric_limits< float >::epsilon() )
				transform.rotation.addX( dt * pitch_change );

			if ( yaw_change > std::numeric_limits< float >::epsilon()
			     || yaw_change < -std::numeric_limits< float >::epsilon() )
				transform.rotation.addY( dt * yaw_change );
		}
		else // No cursor
		{
			const auto pos { getCursorPos( window ) };
			const float xpos { pos.x };
			const float ypos { pos.y };

			UniversalRotation& target_rotation { transform.rotation };

			target_rotation.addZ( ( xpos * 0.006f ) * look_speed );
			target_rotation.addX( ( ypos * 0.006f ) * look_speed );

			setCursorPos( window, { 0, 0 } );
		}

		const Vector forward_dir { transform.rotation.forward() };
		const Vector up_dir { transform.rotation.up() };
		const Vector right_dir { transform.rotation.right() };

		Vector move_dir { 0.0f };
		if ( glfwGetKey( window, key_mappings.move_forward ) == GLFW_PRESS ) move_dir += forward_dir;
		if ( glfwGetKey( window, key_mappings.move_backward ) == GLFW_PRESS ) move_dir -= forward_dir;
		if ( glfwGetKey( window, key_mappings.move_right ) == GLFW_PRESS ) move_dir += right_dir;
		if ( glfwGetKey( window, key_mappings.move_left ) == GLFW_PRESS ) move_dir -= right_dir;
		if ( glfwGetKey( window, key_mappings.move_up ) == GLFW_PRESS ) move_dir += up_dir;
		if ( glfwGetKey( window, key_mappings.move_down ) == GLFW_PRESS ) move_dir -= up_dir;

		const NormalVector n_move_dir { move_dir };

		if ( glm::dot( move_dir.vec(), move_dir.vec() ) > std::numeric_limits< float >::epsilon() )
			transform.translation += n_move_dir * ( move_speed * dt );
	}

} // namespace fgl::engine