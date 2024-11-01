//
// Created by kj16609 on 10/28/24.
//

#pragma once
#include "engine/FGL_DEFINES.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{

	//! Returns the max and min of a point along an axis
	std::pair< float, float > minMaxDot( const NormalVector axis, const auto points )
	{
		assert( points.size() > 2 );
		float min { std::numeric_limits< float >::infinity() };
		float max { -std::numeric_limits< float >::infinity() };

		for ( std::size_t i = 0; i < points.size(); ++i )
		{
			const auto value { glm::dot( points[ i ].vec(), axis.vec() ) };

			min = std::min( min, value );
			max = std::max( max, value );
		}

		return std::make_pair( min, max );
	}

	//! Returns true if we can draw a line between the two point sets.
	template < typename TL, typename TR >
	//TODO: Concept this to only allow for arrays/vectors
	bool testAxis( const NormalVector axis, const TL& left_points, const TR& right_points )
	{
		const auto [ min_left, max_left ] = minMaxDot( axis, left_points );
		const auto [ min_right, max_right ] = minMaxDot( axis, right_points );

		//If there is an overlap then we are unable to draw a line between the two.
		return max_left < min_right || max_right < min_left;
	}

	// Hyperplane Separation Theorem
	// Since the bounding box is oriented we need to test all points from it and 3 axis from the frustum and 3 axis from the box.
	// If we find 1 line that seperates then we fail the test and can early exit
	// This test might be more optimal to do then a pointInside check since it will early fail rather then early pass.
	// We can also make this more optimal by discarding overlapping points. For example, Testing the right axis vector on the Frustum means we can discard the 'low' points. Since the 'high' points would be identical.
	// Though this will likely be a micro-optimizaton in the long run anyways. But less tests means we might be able to almost half the points being tested.

	//! Can plot line between frustum and a given set of points
	template < CoordinateSpace CType, std::uint64_t FrustumPointCount, std::uint64_t BoundingBoxPointCount >
	FGL_FLATTEN_HOT bool canPlotLine(
		const Frustum& frustum,
		const std::array< Coordinate< CType >, FrustumPointCount >& frustum_points,
		const std::array< Coordinate< CType >, BoundingBoxPointCount >& bounding_points )
	{
		return (
			testAxis( frustum.m_right.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.m_left.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.m_near.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.m_far.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.m_top.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.m_bottom.getDirection(), frustum_points, bounding_points ) );
	}

} // namespace fgl::engine