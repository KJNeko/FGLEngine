//
// Created by kj16609 on 8/10/24.
//

#pragma once
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingBox.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine::debug
{

	void drawLine( const WorldCoordinate& p1, const WorldCoordinate& p2 );

	void drawLine( const LineSegment< CoordinateSpace::World >& line );

	void drawBoundingBox( const AxisAlignedBoundingBox< CoordinateSpace::World >& bounding_box );
	void drawBoundingBox( const OrientedBoundingBox< CoordinateSpace::World >& bounding_box );
	void drawFrustum( const Frustum& frustum );

} // namespace fgl::engine::debug