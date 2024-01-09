//
// Created by kj16609 on 11/28/23.
//

#include "KeyboardMovementController.hpp"

#include <functional>

namespace fgl::engine
{

	void KeyboardMovementController::moveInPlaneXZ( GLFWwindow* window, float dt, fgl::engine::GameObject& target )
	{
		glm::vec3 rotate { 0.0f };

		if ( glfwGetKey( window, key_mappings.look_right ) == GLFW_PRESS ) rotate.y += 1.f;
		if ( glfwGetKey( window, key_mappings.look_left ) == GLFW_PRESS ) rotate.y -= 1.f;
		if ( glfwGetKey( window, key_mappings.look_up ) == GLFW_PRESS ) rotate.x += 1.f;
		if ( glfwGetKey( window, key_mappings.look_down ) == GLFW_PRESS ) rotate.x -= 1.f;

		static bool cursor_enabled { true };
		static std::chrono::time_point last_pressed { std::chrono::steady_clock::now() };

		if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
		{
			const auto now { std::chrono::steady_clock::now() };
			using namespace std::chrono_literals;
			if ( now - last_pressed > 1s )
			{
				cursor_enabled = !cursor_enabled;
				last_pressed = now;
			}
		}

		if ( cursor_enabled )
			glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		else
			glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

		if ( !cursor_enabled )
		{
			//Take cursor input
			double xpos { 0.0f };
			double ypos { 0.0f };
			glfwGetCursorPos( window, &xpos, &ypos );

			target.transform.rotation.y +=
				static_cast< float >( ( xpos * 0.006 ) * static_cast< double >( look_speed ) );
			target.transform.rotation.x -=
				static_cast< float >( ( ypos * 0.006 ) * static_cast< double >( look_speed ) );

			glfwSetCursorPos( window, 0, 0 );
		}
		else
		{
			if ( glm::dot( rotate, rotate ) > std::numeric_limits< float >::epsilon() )
				target.transform.rotation += look_speed * dt * glm::normalize( rotate );

			target.transform.rotation.x = glm::clamp( target.transform.rotation.x, -1.5f, 1.5f );
			target.transform.rotation.y = glm::mod( target.transform.rotation.y, glm::two_pi< float >() );
		}

		const float yaw { target.transform.rotation.y };
		const glm::vec3 forward_dir { std::sin( yaw ), 0.0f, std::cos( yaw ) };
		const glm::vec3 right_dir { forward_dir.z, 0.0f, -forward_dir.x };
		const glm::vec3 up_dir { 0.0f, -0.1f, 0.0f };

		glm::vec3 move_dir { 0.0f };
		if ( glfwGetKey( window, key_mappings.move_forward ) == GLFW_PRESS ) move_dir += forward_dir;
		if ( glfwGetKey( window, key_mappings.move_backward ) == GLFW_PRESS ) move_dir -= forward_dir;
		if ( glfwGetKey( window, key_mappings.move_right ) == GLFW_PRESS ) move_dir += right_dir;
		if ( glfwGetKey( window, key_mappings.move_left ) == GLFW_PRESS ) move_dir -= right_dir;
		if ( glfwGetKey( window, key_mappings.move_up ) == GLFW_PRESS ) move_dir += up_dir;
		if ( glfwGetKey( window, key_mappings.move_down ) == GLFW_PRESS ) move_dir -= up_dir;

		if ( glm::dot( move_dir, move_dir ) > std::numeric_limits< float >::epsilon() )
			target.transform.translation += move_speed * dt * glm::normalize( move_dir );
	}

} // namespace fgl::engine