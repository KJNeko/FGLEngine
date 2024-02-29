//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <glm/vec3.hpp>

#include "engine/primitives/CoordinateSpace.hpp"

namespace fgl::engine
{

	template < CoordinateSpace >
	class Coordinate;

	class Vector;

	class NormalVector : public glm::vec3
	{
		constexpr explicit NormalVector( const glm::vec3 point, [[maybe_unused]] const bool ) : glm::vec3( point ) {}

	  public:

		//TODO: Make my own normalize function to bypass the fact glm::normalize can't be constexpr
		NormalVector( const Vector vec );
		explicit NormalVector( const glm::vec3 vec );

		Vector operator*( const float scalar ) const;

		NormalVector operator-() const { return NormalVector( -static_cast< glm::vec3 >( *this ) ); }

		//Used to bypass the glm::normalize constructor
		constexpr static NormalVector bypass( const glm::vec3 point ) { return NormalVector( point, true ); }
	};

} // namespace fgl::engine
