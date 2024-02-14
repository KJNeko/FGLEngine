//
// Created by kj16609 on 2/12/24.
//

#include "Vector.hpp"

namespace fgl::engine
{

	Vector::Vector( const Vector& other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

	Vector& Vector::operator=( const Vector& other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	Vector::Vector( Vector&& other )
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

	Vector& Vector::operator=( const Vector&& other )
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
		return { std::sin( yaw ), std::cos( yaw ), 0.0f };
	}

	glm::vec3 Vector::right() const
	{
		const auto forward_dir { forward() };
		return { -forward_dir.y, forward_dir.x, 0.0f };
	}

} // namespace fgl::engine
