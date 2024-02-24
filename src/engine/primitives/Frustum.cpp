//
// Created by kj16609 on 2/23/24.
//

#include "Frustum.hpp"

#include "engine/model/BoundingBox.hpp"

namespace fgl::engine
{

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const Line< CoordinateSpace::World > line ) const
	{
		const bool top_intersects { top.intersects( line ) };
		const bool bottom_intersects { bottom.intersects( line ) };

		const bool left_intersects { left.intersects( line ) };
		const bool right_intersects { right.intersects( line ) };

		const bool near_intersects { near.intersects( line ) };
		const bool far_intersects { far.intersects( line ) };

		//Check if the line passes through the frustum
		const bool intersects_left_right { left_intersects && right_intersects };
		const bool intersects_top_bottom { top_intersects && bottom_intersects };

		const bool line_within_near_far { !near_intersects && !far_intersects };

		const bool line_outside_top_bottom { !top_intersects && !bottom_intersects };
		const bool line_outside_left_right { !left_intersects && !right_intersects };

		const bool line_outside_range { line_outside_top_bottom && line_outside_left_right };

		return line_within_near_far && !( line_outside_range ) && ( intersects_top_bottom || intersects_left_right );
	}

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const BoundingBox< CoordinateSpace::World > box ) const
	{
		//Check if any of the box's points are inside the frustum
		for ( const auto point : box.points() )
		{
			if ( pointInside( point ) ) return true;
		}

		//Slow check for checking lines
		for ( const auto line : box.lines() )
		{
			if ( intersects( line ) ) return true;
		}

		return false;
	}

} // namespace fgl::engine
