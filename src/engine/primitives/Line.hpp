//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include "Coordinate.hpp"
#include "Vector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct Line
	{
		Coordinate< CType > start;
		Coordinate< CType > end;

		Vector direction() const { return Vector( static_cast< glm::vec3 >( end - start ) ); }

		Line( const Coordinate< CType > start, const Coordinate< CType > end ) : start( start ), end( end ) {}

		Line( const glm::vec3 start, glm::vec3 end ) : start( start ), end( end ) {}

		inline Line< CType > flip() const { return { end, start }; }
	};

} // namespace fgl::engine
