//
// Created by kj16609 on 2/17/24.
//

#pragma once

#include <glm/glm.hpp>

namespace fgl::engine
{
	struct Rotation : public glm::vec3
	{
		float& pitch() { return x; }

		float pitch() const { return x; }

		float& roll() { return y; }

		float roll() const { return z; }

		float& yaw() { return z; }

		float yaw() const { return z; }

		Rotation();

		Rotation( const float value );

		Rotation( const float pitch_r, const float roll_r, const float yaw_r );

		Rotation( const Rotation& other ) = default;

		Rotation& operator=( const Rotation other );

		Rotation& operator+=( const glm::vec3 vec );

		glm::vec3 forward() const;

		glm::vec3 backwards() const;

		glm::vec3 right( const glm::vec3 up ) const;

		glm::vec3 left( const glm::vec3 up ) const;
	};

	namespace constants
	{
		const Rotation DEFAULT_ROTATION { 0.0f, 0.0f, 0.0f };
	}

} // namespace fgl::engine
