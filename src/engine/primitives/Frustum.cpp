//
// Created by kj16609 on 2/23/24.
//

#include "Frustum.hpp"

#include "engine/debug/drawers.hpp"
#include "engine/model/BoundingBox.hpp"
#include "imgui/imgui.h"

namespace fgl::engine
{

	float signedDistance( const Vector direction, const WorldCoordinate& point, const WorldCoordinate& origin )
	{
		const glm::vec3 vector_between { point - origin };

		float dot { glm::dot( vector_between, static_cast< glm::vec3 >( glm::normalize( direction ) ) ) };

		return dot;
	}

	void processPlane(
		const Plane< CoordinateSpace::World > plane,
		const Line< CoordinateSpace::World > line,
		std::vector< WorldCoordinate >& out_enter_intersections,
		std::vector< WorldCoordinate >& out_exit_intersections )
	{
		const WorldCoordinate intersection { plane.intersection( line ) };

		if ( std::isnan( intersection.x ) || std::isnan( intersection.y ) || std::isnan( intersection.z ) ) return;

		//! The line is entering if the line vector is pointing the same direction as the plane's vector
		const bool is_line_entering {
			glm::dot( glm::normalize( line.direction() ), glm::normalize( plane.direction() ) ) > 0.0f
		};

		if ( is_line_entering )
		{
			//			debug::world::drawVector( intersection, line.direction() );
			//			debug::world::drawVector( intersection, plane.direction(), "", glm::vec3( 0.0f, 1.0f, 0.0f ) );
			out_enter_intersections.emplace_back( intersection );
		}
		else
		{
			//			debug::world::drawVector( intersection, line.direction() );
			//			debug::world::drawVector( intersection, plane.direction(), "", glm::vec3( 1.0f, 0.0f, 0.0f ) );
			out_exit_intersections.emplace_back( intersection );
		}
	}

	inline static bool show_intersect { false };

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const Line< CoordinateSpace::World > line ) const
	{
		std::vector< WorldCoordinate > enter_intersections { line.start };
		std::vector< WorldCoordinate > exit_intersections { line.end };

		processPlane( near, line, enter_intersections, exit_intersections );
		processPlane( far, line, enter_intersections, exit_intersections );
		processPlane( left, line, enter_intersections, exit_intersections );
		processPlane( right, line, enter_intersections, exit_intersections );
		processPlane( top, line, enter_intersections, exit_intersections );
		processPlane( bottom, line, enter_intersections, exit_intersections );

		if ( enter_intersections.size() == 0 ) return false;
		if ( exit_intersections.size() == 0 ) return false;

		WorldCoordinate last_enter { enter_intersections.at( 0 ) };
		float last_enter_distance { 0.0f };

		WorldCoordinate first_exit { exit_intersections.at( 0 ) };
		float first_exit_distance { signedDistance( line.direction(), line.end, line.start ) };
		assert( first_exit_distance > 0.0f );

		//Determine the first exit
		for ( const auto exit_point : exit_intersections )
		{
			const float exit_distance { signedDistance( line.direction(), exit_point, line.start ) };

			if ( first_exit_distance > exit_distance )
			{
				//The point happens before the previous exit point
				first_exit_distance = exit_distance;
				first_exit = exit_point;
			}
		}

		for ( const auto enter_point : enter_intersections )
		{
			const float enter_distance { signedDistance( line.direction(), enter_point, line.start ) };

			if ( last_enter_distance < enter_distance )
			{
				last_enter_distance = enter_distance;
				last_enter = enter_point;
			}
		}

		const float distance_to_exit { signedDistance( line.direction(), first_exit, line.start ) };
		const float distance_to_enter { signedDistance( line.direction(), last_enter, line.start ) };

		if ( show_intersect ) [[unlikely]]
		{
			debug::world::drawVector( last_enter, line.direction(), "", glm::vec3( 0.f, 1.f, 0.0f ) );
			debug::world::drawVector( first_exit, line.direction(), "", glm::vec3( 1.f, 0.f, 0.0f ) );
		}

		return distance_to_exit >= distance_to_enter;
	}

	inline static bool check_points { true };
	inline static bool check_lines { true };

	void imGuiFrustumSettings()
	{
		//Check if any of the box's points are inside the frustum
		if ( ImGui::CollapsingHeader( "Frustum intersection settings" ) )
		{
			ImGui::Checkbox( "Check points", &check_points );
			ImGui::Checkbox( "Check lines", &check_lines );
			ImGui::Checkbox( "Show first & last intersections", &show_intersect );
		}
	}

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const BoundingBox< CoordinateSpace::World > box ) const
	{
		if ( check_points )
		{
			for ( const auto point : box.points() )
			{
				if ( pointInside( point ) ) return true;
			}
		}

		if ( check_lines )
		{
			//Slow check for checking lines
			for ( const auto line : box.lines() )
			{
				//intersects( line );
				if ( intersects( line ) ) return true;
			}
		}

		return false;
	}

} // namespace fgl::engine
