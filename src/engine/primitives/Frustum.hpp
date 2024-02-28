//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "Coordinate.hpp"
#include "LineSegment.hpp"
#include "OriginDistancePlane.hpp"
#include "PointPlane.hpp"
#include "engine/primitives/matricies/Matrix.hpp"

namespace fgl::engine
{

	class Camera;

	template < CoordinateSpace CType = CoordinateSpace::World >
	struct Frustum
	{
		Plane< CType > near {};
		Plane< CType > far {};
		Plane< CType > top {};
		Plane< CType > bottom {};
		Plane< CType > right {};
		Plane< CType > left {};

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
		{
			assert( right_plane.direction() != left_plane.direction() );
			assert( top_plane.direction() != bottom_plane.direction() );
			assert( near_plane.direction() != far_plane.direction() );
		}

		Vector forwardVec() const { return near.direction(); }

		Vector upVec() const { return glm::cross( forwardVec(), left.direction() ); }

		Vector rightVec() const { return glm::cross( forwardVec(), upVec() ); }

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
		bool intersects( const T t ) const;

		std::array< Coordinate< CType >, 4 * 2 > points() const
		{
			const Vector pv0 { glm::cross( top.direction(), left.direction() ) };
			const Vector pv1 { glm::cross( top.direction(), right.direction() ) };
			const Vector pv2 { glm::cross( bottom.direction(), left.direction() ) };
			const Vector pv3 { glm::cross( bottom.direction(), right.direction() ) };

			const auto p0 { far.intersection( m_position, pv0 ) };
			const auto p1 { far.intersection( m_position, pv1 ) };
			const auto p2 { far.intersection( m_position, pv2 ) };
			const auto p3 { far.intersection( m_position, pv3 ) };

			const auto p4 { near.intersection( m_position, pv0 ) };
			const auto p5 { near.intersection( m_position, pv1 ) };
			const auto p6 { near.intersection( m_position, pv2 ) };
			const auto p7 { near.intersection( m_position, pv3 ) };

			return { p0, p1, p2, p3, p4, p5, p6, p7 };
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
		Frustum< EvolvedType< MType >() > result {};
		result.near = matrix * frustum.near;
		result.far = matrix * frustum.far;
		result.top = matrix * frustum.top;
		result.bottom = matrix * frustum.bottom;
		result.right = matrix * frustum.right;
		result.left = matrix * frustum.left;
		result.m_position = matrix * frustum.m_position;

		return result;
	}

} // namespace fgl::engine
