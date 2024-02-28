//
// Created by kj16609 on 2/23/24.
//

#include "Frustum.hpp"

#include "engine/debug/drawers.hpp"
#include "engine/model/OrientedBoundingBox.hpp"
#include "imgui/imgui.h"

namespace fgl::engine
{

	float signedDistance( const Vector direction, const WorldCoordinate& point, const WorldCoordinate& origin )
	{
		const glm::vec3 vector_between { point - origin };

		float dot { glm::dot( vector_between, static_cast< glm::vec3 >( glm::normalize( direction ) ) ) };

		assert( !std::isnan( dot ) );
		assert( !std::isinf( dot ) );

		return dot;
	}

	void processPlane(
		const Plane< CoordinateSpace::World > plane,
		const LineSegment< CoordinateSpace::World > line,
		std::vector< WorldCoordinate >& out_enter_intersections,
		std::vector< WorldCoordinate >& out_exit_intersections )
	{
		const WorldCoordinate intersection { plane.intersection( line ) };

		if ( std::isnan( intersection.x ) || std::isnan( intersection.y ) || std::isnan( intersection.z ) ) return;

		const float dot { glm::dot( glm::normalize( line.direction() ), glm::normalize( plane.direction() ) ) };

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
		const std::vector< WorldCoordinate >& exit_intersections, const LineSegment< CoordinateSpace::World > line )
	{
		assert( exit_intersections.size() > 0 );

		WorldCoordinate exit_point { exit_intersections.at( 0 ) };
		float distance { 0.0f };

		for ( const auto intersection_point : exit_intersections )
		{
			const float exit_distance { signedDistance( line.direction(), intersection_point, line.start ) };

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
		const std::vector< WorldCoordinate >& enter_intersections, const LineSegment< CoordinateSpace::World > line )
	{
		assert( enter_intersections.size() > 0 );

		WorldCoordinate first_exit { enter_intersections.at( 0 ) };
		float distance { signedDistance( line.direction(), line.end, line.start ) };
		assert( distance > 0.0f );

		for ( const auto intersection_point : enter_intersections )
		{
			const float enter_distance { signedDistance( line.direction(), intersection_point, line.start ) };

			//If the distance is higher then set it.
			if ( enter_distance > distance )
			{
				distance = enter_distance;
				first_exit = intersection_point;
			}
		}

		return first_exit;
	}

	inline static bool show_intersect { false };

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const LineSegment< CoordinateSpace::World > line ) const
	{
		std::vector< WorldCoordinate > enter_intersections { line.start };
		std::vector< WorldCoordinate > exit_intersections { line.end };

		processPlane( near, line, enter_intersections, exit_intersections );
		processPlane( far, line, enter_intersections, exit_intersections );
		processPlane( left, line, enter_intersections, exit_intersections );
		processPlane( right, line, enter_intersections, exit_intersections );
		processPlane( top, line, enter_intersections, exit_intersections );
		processPlane( bottom, line, enter_intersections, exit_intersections );

		assert( enter_intersections.size() > 1 );
		assert( exit_intersections.size() > 1 );

		if ( enter_intersections.size() == 0 ) return false;
		if ( exit_intersections.size() == 0 ) return false;

		const auto first_exit { getFirstExit( exit_intersections, line ) };
		const auto last_enter { getLastEnter( enter_intersections, line ) };

		const float distance_to_exit { signedDistance( line.direction(), first_exit, line.start ) };
		const float distance_to_enter { signedDistance( line.direction(), last_enter, line.start ) };

		if ( show_intersect ) [[unlikely]]
		{
			debug::world::drawVector(
				last_enter, line.direction(), std::to_string( distance_to_enter ), glm::vec3( 0.f, 1.f, 0.0f ) );
			debug::world::drawVector(
				first_exit, line.direction(), std::to_string( distance_to_exit ), glm::vec3( 1.f, 0.f, 0.0f ) );
		}

		return distance_to_exit >= distance_to_enter;
	}

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const WorldCoordinate t ) const
	{
		return pointInside( t );
	}

	inline static bool check_points { true };
	inline static bool check_lines { true };
	inline static bool check_single_line { false };
	inline static int line_id { 0 };

	void imGuiFrustumSettings()
	{
		//Check if any of the box's points are inside the frustum
		if ( ImGui::CollapsingHeader( "Frustum intersection settings" ) )
		{
			ImGui::Checkbox( "Check points", &check_points );
			ImGui::Checkbox( "Check lines", &check_lines );
			ImGui::Checkbox( "Show first & last intersections", &show_intersect );

			ImGui::Checkbox( "Check single line", &check_single_line );
			if ( check_single_line )
			{
				ImGui::SliderInt( "Line ID:", &line_id, 0, 11 );
				check_points = false;
			}
		}
	}

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const OrientedBoundingBox< CoordinateSpace::World > box ) const
	{
		const auto box_points { box.points() };

		debug::world::drawBoundingBox( box );

		for ( const auto point : box_points )
		{
			if ( pointInside( point ) ) return true;
		}

		//TODO: Do weird line intersection shit

		return true;
	}

} // namespace fgl::engine
