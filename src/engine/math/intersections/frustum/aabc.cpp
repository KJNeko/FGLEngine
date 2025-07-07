//
// Created by kj16609 on 10/28/24.
//

#include "engine/math/intersections/helpers.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingBox.hpp"

namespace fgl::engine::intersections
{

	bool intersects( const Frustum& frustum, const AxisAlignedBoundingBox< CS::World >& aabb )
	{
		const auto box_points { aabb.points() };

		if ( frustum.containsAnyPoint( box_points ) ) return true;

		const auto frustum_points { frustum.points() };

		if ( canPlotLine( frustum, frustum_points, box_points ) ) return false;

		// Now to test every axis from the bounding box
		if ( testAxis( aabb.right(), frustum_points, box_points ) ) return false;
		if ( testAxis( aabb.forward(), frustum_points, box_points ) ) return false;
		if ( testAxis( aabb.up(), frustum_points, box_points ) ) return false;

		return true;
	}

} // namespace fgl::engine::intersections