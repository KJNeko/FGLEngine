//
// Created by kj16609 on 2/23/24.
//

#include "Frustum.hpp"

#include "engine/debug/drawers.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingBox.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"

namespace fgl::engine
{

	float signedDistance( const NormalVector direction, const WorldCoordinate& point, const WorldCoordinate& origin )
	{
		const glm::vec3 vector_between { point.vec() - origin.vec() };

		float dot { glm::dot( vector_between, direction.vec() ) };

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
		const std::vector< WorldCoordinate >& exit_intersections, const LineSegment< CoordinateSpace::World > line )
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
		const std::vector< WorldCoordinate >& enter_intersections, const LineSegment< CoordinateSpace::World > line )
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

	inline static bool show_intersect { false };

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const LineSegment< CoordinateSpace::World >& line ) const
	{
		std::vector< WorldCoordinate > enter_intersections { line.getPosition() };
		std::vector< WorldCoordinate > exit_intersections { line.getEnd() };

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

		const float distance_to_exit { signedDistance( line.getDirection(), first_exit, line.getPosition() ) };
		const float distance_to_enter { signedDistance( line.getDirection(), last_enter, line.getPosition() ) };

		if ( show_intersect ) [[unlikely]]
		{
			debug::world::drawVector(
				last_enter, line.getDirection(), std::to_string( distance_to_enter ), glm::vec3( 0.f, 1.f, 0.0f ) );
			debug::world::drawVector(
				first_exit, line.getDirection(), std::to_string( distance_to_exit ), glm::vec3( 1.f, 0.f, 0.0f ) );
		}

		return distance_to_exit >= distance_to_enter;
	}

	template < CoordinateSpace CType >
	FGL_FORCE_INLINE Vector Frustum< CType >::forwardVec() const
	{
		return near.direction();
	}

	template < CoordinateSpace CType >
	FGL_FORCE_INLINE Vector Frustum< CType >::upVec() const
	{
		return glm::cross( forwardVec(), left.direction() );
	}

	template < CoordinateSpace CType >
	FGL_FORCE_INLINE Vector Frustum< CType >::rightVec() const
	{
		return glm::cross( forwardVec(), upVec() );
	}

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const WorldCoordinate& t ) const
	{
		return pointInside( t );
	}

	//TODO: Implement frustum debug menu
	[[maybe_unused]] inline static bool check_points { true };
	[[maybe_unused]] inline static bool check_lines { true };
	[[maybe_unused]] inline static bool check_single_line { false };
	[[maybe_unused]] inline static int line_id { 0 };

	void imGuiFrustumSettings()
	{
		/*
#if ENABLE_IMGUI
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
#endif
		*/
	}

	template <>
	template <>
	bool Frustum< CoordinateSpace::World >::intersects( const std::vector< WorldCoordinate >& point_cloud ) const
	{
		for ( const auto point : point_cloud )
		{
			if ( intersects( point ) ) return true;
		}

		return false;
	}

	//! Returns the max and min of a point along an axis
	std::pair< float, float > minMaxDot( const NormalVector axis, const auto points )
	{
		assert( points.size() > 2 );
		float min { glm::dot( points[ 0 ].vec(), axis.vec() ) };
		float max { glm::dot( points[ 0 ].vec(), axis.vec() ) };

		for ( std::size_t i = 0; i < points.size(); ++i )
		{
			const auto value { glm::dot( points[ i ].vec(), axis.vec() ) };
			if ( value < min )
				min = value;
			else if ( value > max )
				max = value;
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

	// Dividing Hyperline Theorem
	// Since the bounding box is oriented we need to test all points from it and 3 axis from the frustum and 3 axis from the box.
	// If we find 1 line that seperates then we fail the test and can early exit
	// This test might be more optimal to do then a pointInside check since it will early fail rather then early pass.
	// We can also make this more optimal by discarding overlapping points. For example, Testing the right axis vector on the Frustum means we can discard the 'low' points. Since the 'high' points would be identical.
	// Though this will likely be a micro-optimizaton in the long run anyways. But less tests means we might be able to almost half the points being tested.

	//! Can plot line between frustum and a given set of points
	template < CoordinateSpace CType, std::uint64_t FrustumPointCount, std::uint64_t BoundingBoxPointCount >
	FGL_FLATTEN_HOT bool canPlotLine(
		const Frustum< CType >& frustum,
		const std::array< Coordinate< CType >, FrustumPointCount >& frustum_points,
		const std::array< Coordinate< CType >, BoundingBoxPointCount >& bounding_points )
	{
		return (
			testAxis( frustum.right.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.left.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.near.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.far.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.top.getDirection(), frustum_points, bounding_points )
			|| testAxis( frustum.bottom.getDirection(), frustum_points, bounding_points ) );
	}

	template <>
	template <>
	FGL_FLATTEN bool Frustum< CoordinateSpace::World >::intersects( const OrientedBoundingBox< CoordinateSpace::World >&
	                                                                    bounding_box ) const
	{
		const auto box_points { bounding_box.points() };

		if ( intersects( box_points ) ) return true;

		const auto frustum_points { this->points() };

		if ( canPlotLine( *this, frustum_points, box_points ) ) return false;

		// Now to test every axis from the bounding box
		if ( testAxis( bounding_box.right(), frustum_points, box_points ) ) return false;
		if ( testAxis( bounding_box.forward(), frustum_points, box_points ) ) return false;
		if ( testAxis( bounding_box.up(), frustum_points, box_points ) ) return false;

		return true;
	}

	template <>
	template <>
	FGL_FLATTEN bool Frustum< CoordinateSpace::World >::intersects( const AxisAlignedBoundingBox<
																	CoordinateSpace::World >& bounding_box ) const
	{
		const auto box_points { bounding_box.points() };

		if ( intersects( box_points ) ) return true;

		const auto frustum_points { this->points() };

		if ( canPlotLine( *this, frustum_points, box_points ) ) return false;

		// Now to test every axis from the bounding box
		if ( testAxis( bounding_box.right(), frustum_points, box_points ) ) return false;
		if ( testAxis( bounding_box.forward(), frustum_points, box_points ) ) return false;
		if ( testAxis( bounding_box.up(), frustum_points, box_points ) ) return false;

		return true;
	}

	template <>
	template <>
	FGL_FLATTEN bool Frustum< CoordinateSpace::World >::intersects( const AxisAlignedBoundingCube<
																	CoordinateSpace::World >& bounding_box ) const
	{
		const auto box_points { bounding_box.points() };

		if ( intersects( box_points ) ) return true;

		const auto frustum_points { this->points() };

		if ( canPlotLine( *this, frustum_points, box_points ) ) return false;

		// Now to test every axis from the bounding box
		if ( testAxis( bounding_box.right(), frustum_points, box_points ) ) return false;
		if ( testAxis( bounding_box.forward(), frustum_points, box_points ) ) return false;
		if ( testAxis( bounding_box.up(), frustum_points, box_points ) ) return false;

		return true;
	}

	template <>
	template <>
	Coordinate< CoordinateSpace::World > Frustum<
		CoordinateSpace::World >::intersection( const Line< CoordinateSpace::World >& line ) const
	{
		Coordinate< CoordinateSpace::World > coordinate { line.getEnd() };
		const float max_dist { signedDistance( line.getDirection(), coordinate, line.getPosition() ) };

		//Test each. Whatever the line enters exists first is the point to return
		auto testPlane = [ & ]( const Plane< CoordinateSpace::World >& plane )
		{
			const float old_distance { signedDistance( line.getDirection(), coordinate, line.getPosition() ) };
			const auto intersection { line.intersection( plane ) };

			if ( std::isnan( intersection.x ) || std::isnan( intersection.y ) || std::isnan( intersection.z ) ) return;

			const auto intersection_distance {
				signedDistance( line.getDirection(), intersection, line.getPosition() )
			};

			if ( intersection_distance < 0.0f ) return;
			if ( intersection_distance > max_dist ) return;

			if ( old_distance > intersection_distance ) coordinate = intersection;
		};

		testPlane( this->right );
		testPlane( this->left );
		testPlane( this->near );
		testPlane( this->far );
		testPlane( this->top );
		testPlane( this->bottom );

		return coordinate;
	}

} // namespace fgl::engine
