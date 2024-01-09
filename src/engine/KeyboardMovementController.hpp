//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

namespace fgl::engine
{

	class KeyboardMovementController
	{
	  public:

		struct KeyMappings
		{
			int move_left { GLFW_KEY_A };
			int move_right { GLFW_KEY_D };
			int move_forward { GLFW_KEY_W };
			int move_backward { GLFW_KEY_S };
			int move_up { GLFW_KEY_E };
			int move_down { GLFW_KEY_Q };
			int look_left { GLFW_KEY_LEFT };
			int look_right { GLFW_KEY_RIGHT };
			int look_up { GLFW_KEY_UP };
			int look_down { GLFW_KEY_DOWN };
		} key_mappings;

		float move_speed { 3.0f };
		float look_speed { 1.5f };

		void moveInPlaneXZ( GLFWwindow* window, float dt, GameObject& target );
	};
} // namespace fgl::engine