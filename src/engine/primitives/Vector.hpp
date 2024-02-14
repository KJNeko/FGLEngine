//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "Coordinate.hpp"

namespace fgl::engine
{

	template < CoordinateSpace type >
	class Coordinate;

	class Vector : public glm::vec3
	{
	  public:

		float& roll { y };
		float& pitch { x };
		float& yaw { z };

		explicit Vector( const float value ) : glm::vec3( value ) {}

		explicit Vector( const glm::vec3 vec ) : glm::vec3( vec ) {}

		explicit Vector( const float x, const float y, const float z ) : glm::vec3( x, y, z ) {}

		operator glm::vec4() const { return glm::vec4( static_cast< glm::vec3 >( *this ), 0.0f ); }

		Vector norm() const { return Vector( glm::normalize( static_cast< glm::vec3 >( *this ) ) ); }

		Vector operator*( const float scalar ) const { return Vector( static_cast< glm::vec3 >( *this ) * scalar ); }

		glm::vec3 right() const;
		glm::vec3 forward() const;

		//Copy
		Vector( const Vector& other );
		Vector& operator=( const Vector& other );

		//Move (Should never really move?)
		Vector( Vector&& other );
		Vector& operator=( const Vector&& other );

		Vector& operator=( const std::initializer_list< float > list );
	};

	inline Vector operator-( const Vector vec )
	{
		return Vector( -static_cast< glm::vec3 >( vec ) );
	}

} // namespace fgl::engine

namespace glm
{

	inline glm::vec3 normalize( fgl::engine::Vector vector )
	{
		return glm::normalize( static_cast< glm::vec3 >( vector ) );
	}

	inline glm::vec3 cross( const fgl::engine::Vector vec, const glm::vec3 other )
	{
		return glm::cross( static_cast< glm::vec3 >( vec ), other );
	}

	template < CoordinateSpace CType >
	inline Coordinate< CType > operator+( const Coordinate< CType > lhs, const Vector rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) + static_cast< glm::vec3 >( rhs ) );
	}

} // namespace glm
