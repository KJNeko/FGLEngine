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
		//Check if the line passes through the frustum
		const bool intersects_forward_back { near.intersects( line ) && far.intersects( line ) };
		const bool intersects_left_right { left.intersects( line ) && right.intersects( line ) };
		const bool intersects_top_bottom { top.intersects( line ) && bottom.intersects( line ) };

		//const bool line_within_top_bottom { top.isForward( line ) && bottom.isForward( line ) };
		//const bool line_within_left_right { left.isForward( line ) && right.isForward( line ) };
		//At least one point of the line is within the near and far planes
		const bool line_within_near_far { near.isForward( line ) || far.isForward( line ) };

		return line_within_near_far && ( intersects_forward_back || intersects_top_bottom || intersects_left_right );
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
