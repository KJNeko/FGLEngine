//
// Created by kj16609 on 2/17/24.
//

#include "Rotation.hpp"

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
		static_cast< glm::vec3 >( *this ) = static_cast< glm::vec3 >( other );
		return *this;
	}

	Rotation& Rotation::operator+=( const glm::vec3 vec )
	{
		static_cast< glm::vec3& >( *this ) += vec;
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

} // namespace fgl::engine
