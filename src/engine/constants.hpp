//
// Created by kj16609 on 1/30/24.
//

#pragma once

#include <glm/vec3.hpp>

namespace fgl::engine::constants
{

	constexpr glm::vec3 DEFAULT_VEC3 { std::numeric_limits< float >::max() };

	constexpr glm::vec3 WORLD_CENTER { 0.0f, 0.0f, 0.0f };

	// Z UP
	constexpr glm::vec3 WORLD_UP { 0.0f, 0.0f, 1.0f };
	constexpr glm::vec3 WORLD_DOWN { -WORLD_UP };

	// X RIGHT
	constexpr glm::vec3 WORLD_RIGHT { 1.0f, 0.0f, 0.0f };
	constexpr glm::vec3 WORLD_LEFT { -WORLD_RIGHT };

	// Y FORWARD
	constexpr glm::vec3 WORLD_FORWARD { 0.0f, 1.0f, 0.0f };
	constexpr glm::vec3 WORLD_BACKWARD { -WORLD_FORWARD };

	constexpr float DEFAULT_FLOAT { std::numeric_limits< float >::max() };

	constexpr float NEAR_PLANE { 0.1f };
	constexpr float FAR_PLANE { 100.0f };
	constexpr glm::vec3 CENTER { 0.0f, 0.0f, 0.0f };

} // namespace fgl::engine::constants
