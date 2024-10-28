//
// Created by kj16609 on 10/28/24.
//

#include "engine/math/intersections/helpers.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"

namespace fgl::engine::intersections
{
	bool intersects( const Frustum& frustum, const AxisAlignedBoundingCube< CS::World >& aabc )
	{
		const auto box_points { aabc.points() };

		if ( frustum.containsAnyPoint( box_points ) ) return true;

		const auto frustum_points { frustum.points() };

		if ( canPlotLine( frustum, frustum_points, box_points ) ) return false;

		// Now to test every axis from the bounding box
		if ( testAxis( aabc.right(), frustum_points, box_points ) ) return false;
		if ( testAxis( aabc.forward(), frustum_points, box_points ) ) return false;
		if ( testAxis( aabc.up(), frustum_points, box_points ) ) return false;

		return true;
	}

} // namespace fgl::engine::intersections
