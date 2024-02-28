//
// Created by kj16609 on 2/17/24.
//

#pragma once

#include <glm/glm.hpp>

#include "Vector.hpp"
#include "engine/math/taitBryanMatrix.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/matricies/RotationMatrix.hpp"

namespace fgl::engine
{
	struct Rotation;
}

namespace glm
{
	inline float dot( const fgl::engine::Rotation lhs, const fgl::engine::Rotation rhs );
}

namespace fgl::engine
{

	struct Rotation : protected glm::vec3
	{
		template < int N >
		friend std::tuple< float, float > extract( const Rotation rotation, const RotationOrder order );

		Rotation();

		explicit Rotation( const float value );

		explicit Rotation( const float pitch_r, const float roll_r, const float yaw_r );

		Rotation( const Rotation& other ) = default;

		float& pitch() { return x; }

		float pitch() const { return x; }

		float& roll() { return y; }

		float roll() const { return z; }

		float& yaw() { return z; }

		float yaw() const { return z; }

		Rotation& operator=( const Rotation other );

		Rotation& operator+=( const Rotation vec );

		Vector forward() const;

		Vector right() const;

		Vector up() const;

		RotationMatrix mat() const;

		const glm::vec3& vec3() const { return *this; }

		glm::vec3& vec3() { return *this; }

		friend float glm::dot( const Rotation, const Rotation );
		friend Rotation operator*( const float, const Rotation );
	};

	inline Rotation operator*( const float scalar, const Rotation rot )
	{
		return Rotation( rot.x * scalar, rot.y * scalar, rot.z * scalar );
	}

	// TODO: Make normalize function for roation.
	// To do this I simply just need to take the forward direction and rebuild the Rotation.
	// Using trig

	template < MatrixType MType >
	inline Rotation operator*( const Matrix< MType > mat, const Rotation rot )
	{
		const auto new_forward { mat * rot.forward() };
		const auto new_up { mat * rot.up() };
		const auto new_right { mat * rot.right() };

		return {};
	}

	namespace constants
	{
		constexpr glm::vec3 DEFAULT_ROTATION { 0.0f, 0.0f, 0.0f };
	}

} // namespace fgl::engine

namespace glm
{
	inline float dot( const fgl::engine::Rotation lhs, const fgl::engine::Rotation rhs )
	{
		return dot( static_cast< glm::vec3 >( lhs ), static_cast< glm::vec3 >( rhs ) );
	}

} // namespace glm
