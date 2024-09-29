//
// Created by kj16609 on 2/11/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

#include "engine/FGL_DEFINES.hpp"
#include "engine/constants.hpp"
#include "engine/primitives/CoordinateSpace.hpp"

namespace fgl::engine
{

	template < CoordinateSpace type >
	class Coordinate;

	struct NormalVector;

	//TODO: Make normalized form of Vector
	class Vector : private glm::vec3
	{
	  public:

		using glm::vec3::x;
		using glm::vec3::y;
		using glm::vec3::z;

		const glm::vec3& vec() const { return static_cast< const glm::vec3& >( *this ); }

		glm::vec3& vec() { return static_cast< glm::vec3& >( *this ); }

		constexpr Vector() : glm::vec3( constants::DEFAULT_VEC3 ) {}

		constexpr explicit Vector( const float value ) : glm::vec3( value ) {}

		constexpr explicit Vector( const glm::vec3 i_vec ) : glm::vec3( i_vec ) {}

		explicit Vector( const NormalVector normal_vector );

		constexpr explicit Vector( const float i_x, const float i_y, const float i_z ) : glm::vec3( i_x, i_y, i_z ) {}

		glm::vec3 right( const Vector up = Vector( constants::WORLD_Z ) ) const;
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

		//Coordinate has an operator for vector that's much easier to define. So we just invert the order and use that one.
		template < CoordinateSpace CType >
		FGL_FLATTEN Vector operator+( const Coordinate< CType > coord )
		{
			return coord + *this;
		}

		template < CoordinateSpace CType >
		FGL_FLATTEN Vector operator-( const Coordinate< CType > coord )
		{
			return coord - *this;
		}

	};

	inline Vector operator-( const Vector vec )
	{
		return Vector( -vec.vec() );
	}

} // namespace fgl::engine