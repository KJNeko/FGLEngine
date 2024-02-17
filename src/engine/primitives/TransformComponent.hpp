//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Vector.hpp"
#include "engine/primitives/Matrix.hpp"

namespace fgl::engine
{

	//TransformComponent is always in world space
	struct TransformComponent
	{
		WorldCoordinate translation { constants::DEFAULT_VEC3 };
		glm::vec3 scale { 1.0f, 1.0f, 1.0f };

		struct RotationVector : public glm::vec3
		{
			float& pitch { x };
			float& roll { y };
			float& yaw { z };

			RotationVector& operator=( const RotationVector& other )
			{
				*static_cast< glm::vec3* >( this ) = other;
				return *this;
			}

			RotationVector( const float value ) : glm::vec3( value ) {}

			RotationVector( const float pitch_r, const float roll_r, const float yaw_r ) :
			  glm::vec3( pitch_r, roll_r, yaw_r )
			{}

			RotationVector& operator+=( const glm::vec3 vec )
			{
				glm::vec3::operator+=( vec );
				return *this;
			}

			glm::vec3 forward() const
			{
				//TODO: Figure out how to do this with Z axis bullshit
				return glm::vec3 { glm::sin( yaw ), glm::cos( yaw ), 0.0f };
			}

			glm::vec3 right( const glm::vec3 up ) const { return glm::cross( up, forward() ); }

		} rotation { 0.0f, 0.0f, 0.0f };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		glm::mat4 mat4() const;

		inline Matrix< MatrixType::ModelToWorld > mat() const { return Matrix< MatrixType::ModelToWorld >( mat4() ); }

		glm::mat3 normalMatrix() const;
	};

	using Rotation = TransformComponent::RotationVector;

} // namespace fgl::engine
