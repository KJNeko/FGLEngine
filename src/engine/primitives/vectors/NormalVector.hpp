//
// Created by kj16609 on 2/28/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/Rotation.hpp"

namespace fgl::engine
{

	template < CoordinateSpace >
	class Coordinate;

	class Vector;

	inline constexpr float length( const glm::vec3 vec )
	{
		return std::sqrt( std::pow( vec.x, 2.0f ) + std::pow( vec.y, 2.0f ) + std::pow( vec.z, 2.0f ) );
	}

	inline constexpr glm::vec3 normalize( const glm::vec3 vec )
	{
		if ( std::is_constant_evaluated() )
		{
			const auto len { length( vec ) };
			return glm::vec3( vec.x / len, vec.y / len, vec.z / len );
		}
		else
			return glm::normalize( vec );
	}

	//! A vector that must be a distance of 1
	struct NormalVector : private glm::vec3
	{
		using glm::vec3::x;
		using glm::vec3::y;
		using glm::vec3::z;

		constexpr NormalVector() : glm::vec3( normalize( glm::vec3( 1.0f ) ) ) {}

		explicit NormalVector( const Vector vec );

		constexpr explicit NormalVector( const glm::vec3 vec ) : glm::vec3( normalize( vec ) ) {}

		NormalVector( const NormalVector& other ) = default;
		NormalVector& operator=( const NormalVector& other ) = default;

		const glm::vec3& vec() const { return static_cast< const glm::vec3& >( *this ); }

		glm::vec3& vec() { return static_cast< glm::vec3& >( *this ); }

		Vector operator*( const float scalar ) const;

		NormalVector operator-() const { return NormalVector( -static_cast< glm::vec3 >( *this ) ); }
	};

} // namespace fgl::engine
