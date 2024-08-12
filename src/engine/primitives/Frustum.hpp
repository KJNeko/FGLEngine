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

	template < CoordinateSpace CType = CoordinateSpace::World >
	struct Frustum
	{
		Plane< CType > near { Coordinate< CType >( constants::WORLD_CENTER ),
			                  NormalVector( constants::WORLD_FORWARD ) };
		Plane< CType > far { Coordinate< CType >( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		Plane< CType > top { Coordinate< CType >( constants::WORLD_CENTER ), NormalVector( constants::WORLD_FORWARD ) };
		Plane< CType > bottom { Coordinate< CType >( constants::WORLD_CENTER ),
			                    NormalVector( constants::WORLD_FORWARD ) };
		Plane< CType > right { Coordinate< CType >( constants::WORLD_CENTER ),
			                   NormalVector( constants::WORLD_FORWARD ) };
		Plane< CType > left { Coordinate< CType >( constants::WORLD_CENTER ),
			                  NormalVector( constants::WORLD_FORWARD ) };

		Coordinate< CType > m_position {};

		friend class Camera;

	  public:

		Frustum() = default;

		//TODO: Change this to be, near far, top bottom, left right
		Frustum(
			const Plane< CType > near_plane,
			const Plane< CType > far_plane,
			const Plane< CType > top_plane,
			const Plane< CType > bottom_plane,
			const Plane< CType > right_plane,
			const Plane< CType > left_plane,
			const Coordinate< CType > position ) :
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

		Coordinate< CType > getPosition() const { return m_position; }

		//! Tests if a point is inside of the frustum
		bool pointInside( const WorldCoordinate coord ) const
		{
			static_assert(
				CType == CoordinateSpace::World, "pointInside can only be called on World coordinate Frustums" );

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

		template < typename T >
		bool intersects( const T& t ) const;

		template < typename T, std::size_t TSize >
		inline bool intersects( const std::array< T, TSize >& t ) const
		{
			for ( std::size_t i = 0; i < TSize; ++i )
				if ( intersects( t[ i ] ) ) return true;

			return false;
		}

		template < typename T >
		Coordinate< CType > intersection( const T& t ) const;

		std::array< Coordinate< CType >, 4 * 2 > points() const;
		std::array< LineSegment< CType >, ( ( 4 * 2 ) / 2 ) * 3 > lines() const;
	};

#ifdef EXPOSE_FRUSTUM_INTERNALS
	float signedDistance( const Vector direction, const WorldCoordinate& point, const WorldCoordinate& origin );
	void processPlane(
		const Plane< CoordinateSpace::World > plane,
		const Line< CoordinateSpace::World > line,
		std::vector< WorldCoordinate >& out_enter_intersections,
		std::vector< WorldCoordinate >& out_exit_intersections );

	WorldCoordinate getFirstExit(
		const std::vector< WorldCoordinate >& enter_intersections, const Line< CoordinateSpace::World > line );
	WorldCoordinate getLastEnter(
		const std::vector< WorldCoordinate >& exit_intersections, const Line< CoordinateSpace::World > line );
#endif

	void imGuiFrustumSettings();

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
	Frustum< EvolvedType< MType >() > operator*( const Matrix< MType > matrix, const Frustum< CType > frustum )
	{
		Frustum< EvolvedType< MType >() > result { matrix * frustum.near,      matrix * frustum.far,
			                                       matrix * frustum.top,       matrix * frustum.bottom,
			                                       matrix * frustum.right,     matrix * frustum.left,
			                                       matrix * frustum.m_position };

		return result;
	}

} // namespace fgl::engine
