//
// Created by kj16609 on 8/10/24.
//

#pragma once
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingBox.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine::debug
{

	void drawLine( const WorldCoordinate& p1, const WorldCoordinate& p2, glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

	void drawLine( const LineSegment< CoordinateSpace::World >& line, glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

	void drawBoundingBox( const AxisAlignedBoundingBox< CoordinateSpace::World >& bounding_box );
	void drawBoundingBox( const OrientedBoundingBox< CoordinateSpace::World >& bounding_box );
	void drawFrustum( const Frustum& frustum );

	void drawAxisHelper();

} // namespace fgl::engine::debug