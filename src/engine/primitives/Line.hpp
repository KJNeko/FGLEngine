//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include "Coordinate.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct Line
	{
		Coordinate< CType > start;
		Coordinate< CType > end;

		Line( const Coordinate< CType > start, const Coordinate< CType > end ) : start( start ), end( end ) {}
	};

} // namespace fgl::engine
