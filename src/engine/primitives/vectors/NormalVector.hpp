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

namespace fgl::engine
{

	template < CoordinateSpace >
	class Coordinate;

	class Vector;

	//! A vector that must be a distance of 1
	class NormalVector : private glm::vec3
	{
		constexpr explicit NormalVector( const glm::vec3 point, [[maybe_unused]] const bool ) : glm::vec3( point ) {}

	  public:

		//TODO: Make my own normalize function to bypass the fact glm::normalize can't be constexpr
		NormalVector() : glm::vec3( glm::normalize( glm::vec3( 1.0f ) ) ) {}

		NormalVector( const NormalVector& other ) = default;

		explicit NormalVector( const Vector vec );

		explicit NormalVector( const glm::vec3 vec );

		const glm::vec3& vec() const { return static_cast< const glm::vec3& >( *this ); }

		glm::vec3& vec() { return static_cast< glm::vec3& >( *this ); }

		Vector operator*( const float scalar ) const;

		NormalVector operator-() const { return NormalVector( -static_cast< glm::vec3 >( *this ) ); }

		//Used to bypass the glm::normalize constructor
		constexpr static NormalVector bypass( const glm::vec3 point ) { return NormalVector( point, true ); }
	};

} // namespace fgl::engine
