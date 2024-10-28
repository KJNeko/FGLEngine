//
// Created by kj16609 on 10/28/24.
//

#include "engine/math/intersections/helpers.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"

namespace fgl::engine::intersections
{

	bool intersects( const Frustum& frustum, const OrientedBoundingBox< CS::World >& obb )
	{
		const auto box_points { obb.points() };

		if ( frustum.containsAnyPoint( box_points ) ) return true;

		const auto frustum_points { frustum.points() };

		if ( canPlotLine( frustum, frustum_points, box_points ) ) return false;

		// Now to test every axis from the bounding box
		if ( testAxis( obb.right(), frustum_points, box_points ) ) return false;
		if ( testAxis( obb.forward(), frustum_points, box_points ) ) return false;
		if ( testAxis( obb.up(), frustum_points, box_points ) ) return false;

		return true;
	}

} // namespace fgl::engine::intersections
