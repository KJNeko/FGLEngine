//
// Created by kj16609 on 2/12/24.
//

#include "Vector.hpp"

namespace fgl::engine
{

	Vector::Vector( const Vector& other ) : glm::vec3( other.x, other.y, other.z )
	{}

	Vector& Vector::operator=( const Vector& other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	Vector& Vector::operator=( const std::initializer_list< float > list )
	{
		assert( list.size() == 3 );
		x = *( list.begin() );
		y = *( list.begin() + 1 );
		z = *( list.begin() + 2 );
		return *this;
	}

	glm::vec3 Vector::forward() const
	{
		//TODO: Figure out Z shit
		return static_cast< glm::vec3 >( *this );
	}

	glm::vec3 Vector::right( const Vector up ) const
	{
		const Vector forward_dir { forward() };
		const Vector down { up };

		return glm::cross( down, forward_dir );
	}

	Vector Vector::operator+=( const Vector vector )
	{
		glm::vec3::operator+=( static_cast< glm::vec3 >( vector ) );
		return *this;
	}

	Vector Vector::operator-=( const Vector vector )
	{
		glm::vec3::operator-=( static_cast< glm::vec3 >( vector ) );
		return *this;
	}

} // namespace fgl::engine
