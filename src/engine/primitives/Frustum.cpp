//
// Created by kj16609 on 2/23/24.
//

#include "Frustum.hpp"

#include <tracy/Tracy.hpp>

#include <iosfwd>
#include <vector>

#include "boxes/AxisAlignedBoundingBox.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "lines/InfiniteLine.hpp"
#include "lines/LineSegment.hpp"

namespace fgl::engine
{

	Frustum operator*( const Matrix< MatrixType::ModelToWorld >& matrix, const FrustumBase& frustum )
	{
		const Frustum result { matrix * frustum.m_near,    matrix * frustum.m_far,   matrix * frustum.m_top,
			                   matrix * frustum.m_bottom,  matrix * frustum.m_right, matrix * frustum.m_left,
			                   matrix * frustum.m_position };

		return result;
	}

	FGL_FORCE_INLINE inline NormalVector Frustum::forwardVec() const
	{
		return m_near.getDirection();
	}

	FGL_FORCE_INLINE inline NormalVector Frustum::upVec() const
	{
		return NormalVector( glm::cross( forwardVec().vec(), m_left.getDirection().vec() ) );
	}

	FGL_FORCE_INLINE inline NormalVector Frustum::rightVec() const
	{
		return NormalVector( glm::cross( forwardVec().vec(), upVec().vec() ) );
	}

	bool Frustum::containsPoint( const WorldCoordinate coord ) const
	{
		//Ensure the point we are not trying to test a NaN point
		assert( coord.x != std::numeric_limits< decltype( coord.x ) >::quiet_NaN() );
		assert( coord.y != std::numeric_limits< decltype( coord.y ) >::quiet_NaN() );
		assert( coord.z != std::numeric_limits< decltype( coord.z ) >::quiet_NaN() );

		//TODO: This is a biased approach.
		// Approaches for non-biased:
		// We can either make this non-biased by using a projection from distance shot down the FORWARD vector
		// Or we can use SIMD to check all the planes at once.

		return m_near.isForward( coord ) && m_far.isForward( coord ) && m_bottom.isForward( coord )
		    && m_top.isForward( coord ) && m_right.isForward( coord ) && m_left.isForward( coord );
	}

	bool Frustum::containsAnyPoint( const std::vector< WorldCoordinate >& coords ) const
	{
		for ( const auto coord : coords )
			if ( containsPoint( coord ) ) return true;
		return false;
	}

	bool Frustum::containsAnyPoint( const std::array< WorldCoordinate, interface::BoundingBox::POINT_COUNT >& coords )
		const
	{
		for ( const auto coord : coords )
			if ( containsPoint( coord ) ) return true;
		return false;
	}

	std::array< Coordinate< CoordinateSpace::World >, 4 * 2 > Frustum::points() const
	{
		const NormalVector pv0 { glm::cross( m_top.getDirection().vec(), m_left.getDirection().vec() ) };
		const NormalVector pv1 { glm::cross( m_top.getDirection().vec(), m_right.getDirection().vec() ) };
		const NormalVector pv2 { glm::cross( m_bottom.getDirection().vec(), m_left.getDirection().vec() ) };
		const NormalVector pv3 { glm::cross( m_bottom.getDirection().vec(), m_right.getDirection().vec() ) };

		const auto l0 { InfiniteLine< CoordinateSpace::World >( m_position, pv0 ) };
		const auto l1 { InfiniteLine< CoordinateSpace::World >( m_position, pv1 ) };
		const auto l2 { InfiniteLine< CoordinateSpace::World >( m_position, pv2 ) };
		const auto l3 { InfiniteLine< CoordinateSpace::World >( m_position, pv3 ) };

		const auto p0 { l0.intersection( m_far ) };
		const auto p1 { l1.intersection( m_far ) };
		const auto p2 { l2.intersection( m_far ) };
		const auto p3 { l3.intersection( m_far ) };

		const auto p4 { l0.intersection( m_near ) };
		const auto p5 { l1.intersection( m_near ) };
		const auto p6 { l2.intersection( m_near ) };
		const auto p7 { l3.intersection( m_near ) };

		return { { p0, p1, p2, p3, p4, p5, p6, p7 } };
	}

	std::array< LineSegment< CoordinateSpace::World >, ( ( 4 * 2 ) / 2 ) * 3 > Frustum::lines() const
	{
		const auto points { this->points() };

		std::array< LineSegment< CoordinateSpace::World >, ( ( 4 * 2 ) / 2 ) * 3 > lines {};

		//Top
		lines[ 0 ] = LineSegment< CoordinateSpace::World >( points[ 0 ], points[ 1 ] );
		lines[ 1 ] = LineSegment< CoordinateSpace::World >( points[ 1 ], points[ 2 ] );
		lines[ 2 ] = LineSegment< CoordinateSpace::World >( points[ 2 ], points[ 3 ] );
		lines[ 3 ] = LineSegment< CoordinateSpace::World >( points[ 3 ], points[ 0 ] );

		//Bottom
		lines[ 4 ] = LineSegment< CoordinateSpace::World >( points[ 4 ], points[ 5 ] );
		lines[ 5 ] = LineSegment< CoordinateSpace::World >( points[ 5 ], points[ 6 ] );
		lines[ 6 ] = LineSegment< CoordinateSpace::World >( points[ 6 ], points[ 7 ] );
		lines[ 7 ] = LineSegment< CoordinateSpace::World >( points[ 7 ], points[ 4 ] );

		//Sides
		lines[ 8 ] = LineSegment< CoordinateSpace::World >( points[ 0 ], points[ 4 ] );
		lines[ 9 ] = LineSegment< CoordinateSpace::World >( points[ 1 ], points[ 5 ] );
		lines[ 10 ] = LineSegment< CoordinateSpace::World >( points[ 2 ], points[ 6 ] );
		lines[ 11 ] = LineSegment< CoordinateSpace::World >( points[ 3 ], points[ 7 ] );

		return lines;
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

	namespace debug
	{
		void drawFrustum( const Frustum& frustum )
		{
			for ( const auto& line : frustum.lines() )
			{
				debug::drawLine( line );
			}
		}
	} // namespace debug

} // namespace fgl::engine
