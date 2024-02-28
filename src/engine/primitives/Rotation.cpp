//
// Created by kj16609 on 2/17/24.
//

#include "Rotation.hpp"

#include "engine/math/taitBryanMatrix.hpp"

namespace fgl::engine
{

	Rotation::Rotation() : glm::vec3( 0.0f )
	{}

	Rotation::Rotation( const float value ) : glm::vec3( value )
	{}

	Rotation::Rotation( const float pitch_r, const float roll_r, const float yaw_r ) :
	  glm::vec3( pitch_r, roll_r, yaw_r )
	{}

	Rotation& Rotation::operator=( const Rotation other )
	{
		glm::vec3::operator=( other );
		return *this;
	}

	Rotation& Rotation::operator+=( const Rotation i_vec )
	{
		glm::vec3::operator+=( static_cast< glm::vec3 >( i_vec ) );
		return *this;
	}

	Vector Rotation::forward() const
	{
		return mat() * Vector( constants::WORLD_FORWARD );
	}

	Vector Rotation::right() const
	{
		return mat() * Vector( constants::WORLD_RIGHT );
	}

	Vector Rotation::up() const
	{
		return mat() * Vector( constants::WORLD_UP );
	}

	RotationMatrix Rotation::mat() const
	{
		return taitBryanMatrix( *this );
	}

} // namespace fgl::engine
