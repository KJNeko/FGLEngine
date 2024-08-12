//
// Created by kj16609 on 8/12/24.
//

#include "drawers.hpp"

#include <array>

#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "engine/primitives/lines/LineSegment.hpp"

namespace fgl::engine::debug
{

	void drawLine( const LineSegment< CoordinateSpace::World >& line )
	{
		drawLine( line.getStart(), line.getEnd() );
	}

	void drawBoundingBox( const AxisAlignedBoundingBox< CoordinateSpace::World >& bounding_box )
	{
		for ( const auto& line : bounding_box.lines() )
		{
			drawLine( line.getStart(), line.getEnd() );
		}
	}

	void drawBoundingBox( const OrientedBoundingBox< CoordinateSpace::World >& bounding_box )
	{
		for ( const auto& line : bounding_box.lines() )
		{
			drawLine( line.getStart(), line.getEnd() );
		}
	}
} // namespace fgl::engine::debug
