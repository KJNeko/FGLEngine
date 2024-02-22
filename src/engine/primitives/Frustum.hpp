//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "Coordinate.hpp"
#include "Line.hpp"
#include "Matrix.hpp"
#include "Plane.hpp"

namespace fgl::engine
{

	class Camera;

	template < CoordinateSpace CType = CoordinateSpace::World >
	struct Frustum
	{
		Plane< CType > near;
		Plane< CType > far;

		Plane< CType > top;
		Plane< CType > bottom;

		Plane< CType > right;
		Plane< CType > left;

		friend class Camera;

	  public:

		Frustum() = default;

		Frustum(
			const Plane< CType > near_plane,
			const Plane< CType > far_plane,
			const Plane< CType > top_plane,
			const Plane< CType > bottom_plane,
			const Plane< CType > right_plane,
			const Plane< CType > left_plane ) :
		  near( near_plane ),
		  far( far_plane ),
		  top( top_plane ),
		  bottom( bottom_plane ),
		  right( right_plane ),
		  left( left_plane )
		{
			assert( near.valid() );
			assert( far.valid() );
			assert( top.valid() );
			assert( bottom.valid() );
			assert( right.valid() );
			assert( left.valid() );

			assert( right_plane.direction() != left_plane.direction() );
			assert( top_plane.direction() != bottom_plane.direction() );
			assert( near_plane.direction() != far_plane.direction() );
		}

		//! Tests if a point is inside of the frustum
		bool pointInside( const WorldCoordinate coord ) const
		{
			static_assert(
				CType == CoordinateSpace::World, "pointInside can only be called on World coordinate Frustums" );

			//TODO: This is a biased approach.
			// Approaches for non-biased:
			// We can either make this non-biased by using a projection from distance shot down the FORWARD vector
			// Or we can use SIMD to check all the planes at once.

			return far.isForward( coord ) && near.isForward( coord ) && bottom.isForward( coord )
			    && top.isForward( coord ) && right.isForward( coord ) && left.isForward( coord );
		}

		//! Tests that a line intersects the frustum
		bool lineIntersects( const Line< CoordinateSpace::World > line ) const;

		bool operator==( const Frustum< CType >& other ) const
		{
			return near == other.near && far == other.far && top == other.top && bottom == other.bottom
			    && right == other.right && left == other.left;
		}
	};

	template < CoordinateSpace CType >
	inline std::ostream& operator<<( std::ostream& os, const Frustum< CType >& frustum )
	{
		os << "Frustum: " << std::endl;
		os << "\tNear: " << frustum.near << std::endl;
		os << "\tFar: " << frustum.far << std::endl;
		os << "\tTop: " << frustum.top << std::endl;
		os << "\tBottom: " << frustum.bottom << std::endl;
		os << "\tRight: " << frustum.right << std::endl;
		os << "\tLeft: " << frustum.left << std::endl;
		return os;
	}

	template < CoordinateSpace CType, MatrixType MType >
	Frustum< EvolvedType< MType >() > operator*( const Matrix< MType >& matrix, const Frustum< CType >& frustum )
	{
		Frustum< EvolvedType< MType >() > result {};
		result.near = matrix * frustum.near;
		result.far = matrix * frustum.far;
		result.top = matrix * frustum.top;
		result.bottom = matrix * frustum.bottom;
		result.right = matrix * frustum.right;
		result.left = matrix * frustum.left;

		return result;
	}

} // namespace fgl::engine
