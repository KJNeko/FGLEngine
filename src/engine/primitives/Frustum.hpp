//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "engine/primitives/lines/InfiniteLine.hpp"
#include "engine/primitives/planes/OriginDistancePlane.hpp"
#include "engine/primitives/planes/PointPlane.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{

	class Camera;

	template < CoordinateSpace CType = CoordinateSpace::World >
	struct Frustum
	{
		Plane< CType > near { Coordinate< CType >( constants::WORLD_CENTER ),
			                  NormalVector::bypass( constants::WORLD_FORWARD ) };
		Plane< CType > far { Coordinate< CType >( constants::WORLD_CENTER ),
			                 NormalVector::bypass( constants::WORLD_FORWARD ) };
		Plane< CType > top { Coordinate< CType >( constants::WORLD_CENTER ),
			                 NormalVector::bypass( constants::WORLD_FORWARD ) };
		Plane< CType > bottom { Coordinate< CType >( constants::WORLD_CENTER ),
			                    NormalVector::bypass( constants::WORLD_FORWARD ) };
		Plane< CType > right { Coordinate< CType >( constants::WORLD_CENTER ),
			                   NormalVector::bypass( constants::WORLD_FORWARD ) };
		Plane< CType > left { Coordinate< CType >( constants::WORLD_CENTER ),
			                  NormalVector::bypass( constants::WORLD_FORWARD ) };

		Coordinate< CType > m_position {};

		friend class Camera;

	  public:

		Frustum() = default;

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

		Vector FGL_FORCE_INLINE forwardVec() const;

		Vector FGL_FORCE_INLINE upVec() const;

		Vector FGL_FORCE_INLINE rightVec() const;

		Coordinate< CType > getPosition() const { return m_position; }

		//! Tests if a point is inside of the frustum
		bool pointInside( const WorldCoordinate coord ) const
		{
			static_assert(
				CType == CoordinateSpace::World, "pointInside can only be called on World coordinate Frustums" );

			//TODO: This is a biased approach.
			// Approaches for non-biased:
			// We can either make this non-biased by using a projection from distance shot down the FORWARD vector
			// Or we can use SIMD to check all the planes at once.

			return near.isForward( coord ) && far.isForward( coord ) && bottom.isForward( coord )
			    && top.isForward( coord ) && right.isForward( coord ) && left.isForward( coord );
		}

		template < typename T >
		bool intersects( const T& t ) const;

		std::array< Coordinate< CType >, 4 * 2 > points() const
		{
			const Vector pv0 { glm::cross( top.getDirection().vec(), left.getDirection().vec() ) };
			const Vector pv1 { glm::cross( top.getDirection().vec(), right.getDirection().vec() ) };
			const Vector pv2 { glm::cross( bottom.getDirection().vec(), left.getDirection().vec() ) };
			const Vector pv3 { glm::cross( bottom.getDirection().vec(), right.getDirection().vec() ) };

			const auto l0 { InfiniteLine< CoordinateSpace::World >( m_position, pv0 ) };
			const auto l1 { InfiniteLine< CoordinateSpace::World >( m_position, pv1 ) };
			const auto l2 { InfiniteLine< CoordinateSpace::World >( m_position, pv2 ) };
			const auto l3 { InfiniteLine< CoordinateSpace::World >( m_position, pv3 ) };

			const auto p0 { l0.intersection( far ) };
			const auto p1 { l1.intersection( far ) };
			const auto p2 { l2.intersection( far ) };
			const auto p3 { l3.intersection( far ) };

			const auto p4 { l0.intersection( near ) };
			const auto p5 { l1.intersection( near ) };
			const auto p6 { l2.intersection( near ) };
			const auto p7 { l3.intersection( near ) };

			return { { p0, p1, p2, p3, p4, p5, p6, p7 } };
		}

		bool operator==( const Frustum< CType >& other ) const
		{
			return near == other.near && far == other.far && top == other.top && bottom == other.bottom
			    && right == other.right && left == other.left;
		}
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
	Frustum< EvolvedType< MType >() > operator*( const Matrix< MType >& matrix, const Frustum< CType >& frustum )
	{
		Frustum< EvolvedType< MType >() > result { matrix * frustum.near,      matrix * frustum.far,
			                                       matrix * frustum.top,       matrix * frustum.bottom,
			                                       matrix * frustum.right,     matrix * frustum.left,
			                                       matrix * frustum.m_position };

		return result;
	}

} // namespace fgl::engine
