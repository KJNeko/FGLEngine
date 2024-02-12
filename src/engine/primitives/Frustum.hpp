//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "Coordinate.hpp"
#include "Matrix.hpp"
#include "Plane.hpp"

namespace fgl::engine
{

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

		template < MatrixType MType >
		Frustum< EvolvedType< MType >() > operator*( Matrix< MType > matrix ) const
		{
			Frustum< EvolvedType< MType >() > result = *this;
			result.near = near * matrix;
			result.far = far * matrix;
			result.top = top * matrix;
			result.bottom = bottom * matrix;
			result.right = right * matrix;
			result.left = left * matrix;

			return result;
		}

		bool pointInside( const WorldCoordinate& coord ) const
		{
			// clang-format off
			return
				near.isForward( coord ) && far.isForward( coord )
				&& top.isForward( coord ) && bottom.isForward( coord )
				&& right.isForward( coord ) && left.isForward( coord );
			// clang-format on
		}
	};

	template < CoordinateSpace CType, MatrixType MType >
	Frustum< EvolvedType< MType >() > operator*( const Matrix< MType >& matrix, const Frustum< CType >& frustum )
	{
		return frustum * matrix;
	}

} // namespace fgl::engine
