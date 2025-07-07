//
// Created by kj16609 on 10/28/24.
//

#include "engine/primitives/Frustum.hpp"

#include <vector>

#include "engine/primitives/lines/LineSegment.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine::intersection
{

	float signedDistance( const NormalVector direction, const WorldCoordinate& point, const WorldCoordinate& origin )
	{
		const glm::vec3 vector_between { point.vec() - origin.vec() };

		const float dot { glm::dot( vector_between, direction.vec() ) };

		if ( std::isnan( dot ) ) return 0.0f;

		assert( !std::isinf( dot ) );

		return dot;
	}

	void processPlane(
		const Plane< CoordinateSpace::World >& plane,
		const LineSegment< CoordinateSpace::World >& line,
		std::vector< WorldCoordinate >& out_enter_intersections,
		std::vector< WorldCoordinate >& out_exit_intersections )
	{
		const WorldCoordinate intersection { line.intersection( plane ) };

		if ( std::isnan( intersection.vec().x ) || std::isnan( intersection.vec().y )
		     || std::isnan( intersection.vec().z ) )
			return;

		const float dot { glm::dot( line.getDirection().vec(), plane.getDirection().vec() ) };

		assert( !std::isnan( dot ) );
		assert( !std::isinf( dot ) );

		//! The line is entering if the line vector is pointing the same direction as the plane's vector
		const bool is_line_entering { dot > 0.0f };

		if ( is_line_entering )
			out_enter_intersections.emplace_back( intersection );
		else
			out_exit_intersections.emplace_back( intersection );

		return;
	}

	WorldCoordinate getFirstExit(
		const std::vector< WorldCoordinate >& exit_intersections, const LineSegment< CoordinateSpace::World >& line )
	{
		assert( exit_intersections.size() > 0 );

		WorldCoordinate exit_point { exit_intersections.at( 0 ) };
		float distance { 0.0f };

		for ( const auto intersection_point : exit_intersections )
		{
			const float exit_distance { signedDistance( line.getDirection(), intersection_point, line.getPosition() ) };

			//if the distance is lower then it's before the previous
			if ( exit_distance < distance )
			{
				distance = exit_distance;
				exit_point = intersection_point;
			}
		}

		return exit_point;
	}

	WorldCoordinate getLastEnter(
		const std::vector< WorldCoordinate >& enter_intersections, const LineSegment< CoordinateSpace::World >& line )
	{
		assert( enter_intersections.size() > 0 );

		WorldCoordinate first_exit { enter_intersections.at( 0 ) };
		float distance { signedDistance( line.getDirection(), line.getEnd(), line.getPosition() ) };
		//assert( distance > 0.0f );

		for ( const auto intersection_point : enter_intersections )
		{
			const float enter_distance {
				signedDistance( line.getDirection(), intersection_point, line.getPosition() )
			};

			//If the distance is higher then set it.
			if ( enter_distance > distance )
			{
				distance = enter_distance;
				first_exit = intersection_point;
			}
		}

		return first_exit;
	}

	bool intersects( const LineSegment< CS::World >& line, const Frustum& frustum )
	{
		std::vector< WorldCoordinate > enter_intersections { line.getPosition() };
		std::vector< WorldCoordinate > exit_intersections { line.getEnd() };

		processPlane( frustum.m_near, line, enter_intersections, exit_intersections );
		processPlane( frustum.m_far, line, enter_intersections, exit_intersections );
		processPlane( frustum.m_left, line, enter_intersections, exit_intersections );
		processPlane( frustum.m_right, line, enter_intersections, exit_intersections );
		processPlane( frustum.m_top, line, enter_intersections, exit_intersections );
		processPlane( frustum.m_bottom, line, enter_intersections, exit_intersections );

		if ( enter_intersections.size() == 0 ) return false;
		if ( exit_intersections.size() == 0 ) return false;

		const auto first_exit { getFirstExit( exit_intersections, line ) };
		const auto last_enter { getLastEnter( enter_intersections, line ) };

		const float distance_to_exit { signedDistance( line.getDirection(), first_exit, line.getPosition() ) };
		const float distance_to_enter { signedDistance( line.getDirection(), last_enter, line.getPosition() ) };

		return distance_to_exit >= distance_to_enter;
	}

} // namespace fgl::engine::intersection
