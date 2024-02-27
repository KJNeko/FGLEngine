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

	Rotation& Rotation::operator+=( const glm::vec3 i_vec )
	{
		static_cast< glm::vec3& >( *this ) += i_vec;
		return *this;
	}

	glm::vec3 Rotation::forward() const
	{
		//TODO: Figure out how to do this with Z axis bullshit
		return glm::vec3 { glm::sin( yaw() ), glm::cos( yaw() ), 0.0f };
	}

	glm::vec3 Rotation::backwards() const
	{
		return -forward();
	}

	glm::vec3 Rotation::right( const glm::vec3 up ) const
	{
		return glm::cross( -up, forward() );
	}

	glm::vec3 Rotation::left( const glm::vec3 up ) const
	{
		return -right( up );
	}

	glm::mat4 Rotation::mat4() const
	{
		return taitBryanMatrix( *this );
	}

} // namespace fgl::engine
