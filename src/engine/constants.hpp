//
// Created by kj16609 on 1/30/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

namespace fgl::engine::constants
{

	constexpr glm::vec3 DEFAULT_VEC3 { std::numeric_limits< float >::signaling_NaN() };

	constexpr glm::vec3 WORLD_CENTER { 0.0f, 0.0f, 0.0f };

	// X RIGHT
	constexpr glm::vec3 WORLD_X { 1.0f, 0.0f, 0.0f };
	constexpr glm::vec3 WORLD_X_NEG { -WORLD_X };

	// Y FORWARD
	constexpr glm::vec3 WORLD_Y { 0.0f, 1.0f, 0.0f };
	constexpr glm::vec3 WORLD_Y_NEG { -WORLD_Y };

	// Z UP
	constexpr glm::vec3 WORLD_Z { 0.0f, 0.0f, 1.0f };
	constexpr glm::vec3 WORLD_Z_NEG { -WORLD_Z };

	constexpr float DEFAULT_FLOAT { std::numeric_limits< float >::max() };

	constexpr float NEAR_PLANE { 0.1f };
	constexpr float FAR_PLANE { 1000.0f };
	constexpr glm::vec3 CENTER { 0.0f, 0.0f, 0.0f };

	constexpr glm::mat4 MAT4_IDENTITY { 1.0f };
	constexpr glm::mat3 MAT3_IDENTITY { 1.0f };

	constexpr glm::vec3 WORLD_FORWARD { WORLD_X };
	constexpr glm::vec3 WORLD_RIGHT { WORLD_Y };
	constexpr glm::vec3 WORLD_UP { WORLD_Z };
	constexpr glm::vec3 WORLD_BACKWARD { -WORLD_FORWARD };
	constexpr glm::vec3 WORLD_LEFT { -WORLD_RIGHT };
	constexpr glm::vec3 WORLD_DOWN { -WORLD_UP };

	constexpr TextureID INVALID_TEXTURE_ID { 0 };

	using FrameIndex = std::uint8_t;
	constexpr FrameIndex MAX_FRAMES_IN_FLIGHT { 2 };

	constexpr glm::vec3 DEFAULT_SCALE { 1.0f };

	//! Number of items the transfer buffer should attempt to transfer at once
	constexpr std::size_t TRANSFER_LIMIT { 128 };

} // namespace fgl::engine::constants
