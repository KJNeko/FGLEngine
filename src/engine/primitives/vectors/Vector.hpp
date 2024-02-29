//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <glm/vec3.hpp>

#include "engine/constants.hpp"
#include "engine/primitives/CoordinateSpace.hpp"

namespace fgl::engine
{

	template < CoordinateSpace type >
	class Coordinate;

	//TODO: Make normalized form of Vector
	class Vector : public glm::vec3
	{
	  public:

		constexpr Vector() : glm::vec3( constants::DEFAULT_VEC3 ) {}

		constexpr explicit Vector( const float value ) : glm::vec3( value ) {}

		constexpr explicit Vector( const glm::vec3 i_vec ) : glm::vec3( i_vec ) {}

		constexpr explicit Vector( const float i_x, const float i_y, const float i_z ) : glm::vec3( i_x, i_y, i_z ) {}

		glm::vec3 right( const Vector up = Vector( constants::WORLD_UP ) ) const;
		glm::vec3 forward() const;

		Vector operator*( const float scalar ) const { return Vector( static_cast< glm::vec3 >( *this ) * scalar ); }

		Vector operator+=( const Vector );
		Vector operator-=( const Vector );

		//Copy
		Vector( const Vector& other );
		Vector& operator=( const Vector& other );

		//Move (Should never really move?)
		Vector( Vector&& other ) = delete;
		Vector& operator=( const Vector&& other ) = delete;

		Vector& operator=( const std::initializer_list< float > list );
	};

	inline Vector operator-( const Vector vec )
	{
		return Vector( -static_cast< glm::vec3 >( vec ) );
	}

} // namespace fgl::engine

namespace glm
{

	inline fgl::engine::Vector normalize( fgl::engine::Vector vector )
	{
		return fgl::engine::Vector( glm::normalize( static_cast< glm::vec3 >( vector ) ) );
	}

	inline fgl::engine::Vector cross( const fgl::engine::Vector vec, const glm::vec3 other )
	{
		return fgl::engine::Vector( cross( static_cast< vec3 >( vec ), other ) );
	}

	inline fgl::engine::Vector cross( const fgl::engine::Vector vec, const fgl::engine::Vector other )
	{
		return fgl::engine::Vector( cross( static_cast< vec3 >( vec ), static_cast< vec3 >( other ) ) );
	}

	inline float dot( const fgl::engine::Vector a, const fgl::engine::Vector b )
	{
		return dot( static_cast< vec3 >( a ), static_cast< vec3 >( b ) );
	}

} // namespace glm
