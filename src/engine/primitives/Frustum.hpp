//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <ostream>

#include "engine/primitives/planes/PointPlane.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "vectors/Vector.hpp"

namespace fgl::engine
{

	class Camera;

	//! Frustum constructed in model space (To be translated to a World space frustum later)
	struct FrustumBase
	{
		ModelPlane near { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		ModelPlane far { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		ModelPlane top { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		ModelPlane bottom { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		ModelPlane right { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		ModelPlane left { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };

		ModelCoordinate m_position {};

		FrustumBase() = default;

		FrustumBase(
			const ModelPlane& near_plane,
			const ModelPlane& far_plane,
			const ModelPlane& top_plane,
			const ModelPlane& bottom_plane,
			const ModelPlane& right_plane,
			const ModelPlane& left_plane,
			const ModelCoordinate position ) :
		  near( near_plane ),
		  far( far_plane ),
		  top( top_plane ),
		  bottom( bottom_plane ),
		  right( right_plane ),
		  left( left_plane ),
		  m_position( position )
		{}
	};

	struct Frustum
	{
		WorldPlane near { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		WorldPlane far { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		WorldPlane top { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		WorldPlane bottom { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		WorldPlane right { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		WorldPlane left { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };

		WorldCoordinate m_position {};

		friend class Camera;

	  public:

		Frustum() = default;

		//TODO: Change this to be, near far, top bottom, left right
		Frustum(
			const WorldPlane& near_plane,
			const WorldPlane& far_plane,
			const WorldPlane& top_plane,
			const WorldPlane& bottom_plane,
			const WorldPlane& right_plane,
			const WorldPlane& left_plane,
			const WorldCoordinate position ) :
		  near( near_plane ),
		  far( far_plane ),
		  top( top_plane ),
		  bottom( bottom_plane ),
		  right( right_plane ),
		  left( left_plane ),
		  m_position( position )
		{}

		FGL_FORCE_INLINE NormalVector forwardVec() const;
		FGL_FORCE_INLINE NormalVector upVec() const;
		FGL_FORCE_INLINE NormalVector rightVec() const;

		WorldCoordinate getPosition() const { return m_position; }

		//! Tests if a point is inside of the frustum
		bool pointInside( const WorldCoordinate coord ) const
		{
			static_assert(
				CoordinateSpace::World == CoordinateSpace::World,
				"pointInside can only be called on World coordinate Frustums" );

			//Ensure the point we are not trying to test a NaN point
			assert( coord.x != std::numeric_limits< decltype( coord.x ) >::quiet_NaN() );
			assert( coord.y != std::numeric_limits< decltype( coord.y ) >::quiet_NaN() );
			assert( coord.z != std::numeric_limits< decltype( coord.z ) >::quiet_NaN() );

			//TODO: This is a biased approach.
			// Approaches for non-biased:
			// We can either make this non-biased by using a projection from distance shot down the FORWARD vector
			// Or we can use SIMD to check all the planes at once.

			return near.isForward( coord ) && far.isForward( coord ) && bottom.isForward( coord )
			    && top.isForward( coord ) && right.isForward( coord ) && left.isForward( coord );
		}

		//! Returns true if the frustum intersects type T
		template < typename T >
		bool intersects( const T& t ) const;

		//! Itterates over all items in the array given, Returns false if **ANY** fail
		template < typename T, std::size_t N >
		bool intersects( const std::array< T, N >& ts ) const
		{
			for ( const auto& t : ts )
				if ( !intersects( t ) ) return false;
			return true;
		}

		std::array< WorldCoordinate, 4 * 2 > points() const;
		std::array< LineSegment< CoordinateSpace::World >, ( ( 4 * 2 ) / 2 ) * 3 > lines() const;
	};

#ifdef EXPOSE_FRUSTUM_INTERNALS
	float signedDistance( const Vector direction, const WorldCoordinate& point, const WorldCoordinate& origin );
	void processPlane(
		const WorldPlane plane,
		const Line< CoordinateSpace::World > line,
		std::vector< WorldCoordinate >& out_enter_intersections,
		std::vector< WorldCoordinate >& out_exit_intersections );

	WorldCoordinate getFirstExit(
		const std::vector< WorldCoordinate >& enter_intersections, const Line< CoordinateSpace::World > line );
	WorldCoordinate getLastEnter(
		const std::vector< WorldCoordinate >& exit_intersections, const Line< CoordinateSpace::World > line );
#endif

	void imGuiFrustumSettings();

	Frustum operator*( const Matrix< MatrixType::ModelToWorld >& matrix, const FrustumBase& frustum );

} // namespace fgl::engine
