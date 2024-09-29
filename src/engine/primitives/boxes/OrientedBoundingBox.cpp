//
// Created by kj16609 on 1/27/24.
//

#include "OrientedBoundingBox.hpp"

#include <tracy/Tracy.hpp>

#include <array>

#include "AxisAlignedBoundingBox.hpp"
#include "engine/assets/model/ModelVertex.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/primitives/lines/LineSegment.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	consteval std::array< std::uint32_t, OrientedBoundingBox< CType >::indicies_count > OrientedBoundingBox<
		CType >::triangleIndicies()
	{
		/**
		 * Order (Top)
		 * 1 =========== 0
		 * ||			||
		 * ||			||
		 * 2 =========== 3
		 *
		 * Order (Bottom)
		 * 5 =========== 4
		 * ||			||
		 * ||			||
		 * 6 =========== 7
		 *
		 * Order (Side - 1)
		 * 2 =========== 3
		 * ||			||
		 * ||			||
		 * 6 =========== 7
		 *
		 * Order (Side - 2)
		 * 1 =========== 0
		 * ||			||
		 * ||			||
		 * 5 =========== 4
		 *
		 * Order (Side - 3)
		 * 3 =========== 0
		 * ||			||
		 * ||			||
		 * 7 =========== 4
		 *
		 * Order (Side - 4)
		 * 2 =========== 1
		 * ||			||
		 * ||			||
		 * 6 =========== 5
		 */

		// clang-format off
		std::array< std::uint32_t, indicies_count > data {
			{
			//Top tris
			0, 1, 2,
			1, 2, 3,
			//Bottom tris
			4, 5, 6,
			5, 6, 7,
			//Side 1
			2, 3, 6,
			3, 6, 7,
			//Side 2
			1, 0, 5,
			0, 5, 4,
			//Side 3
			3, 0, 7,
			0, 7, 4,
			//Side 4
			2, 1, 6,
			1, 6, 5 }
		};
		// clang-format on

		return data;
	}

	template < CoordinateSpace CType >
	std::array< Coordinate< CType >, interface::BoundingBox::POINT_COUNT > OrientedBoundingBox< CType >::points() const
	{
		// xp == x positive (Highest x point)
		// xn == x negative (Lowest x point)

		//const glm::vec3 xp_yp_zp { m_transform.translation.vec() + static_cast< glm::vec3 >( m_transform.scale ) };
		//const glm::vec3 xn_yn_zn { m_transform.translation.vec() - static_cast< glm::vec3 >( m_transform.scale ) };

		constexpr glm::vec3 xp_yp_zp { 1.0f, 1.0f, 1.0f };
		constexpr glm::vec3 xn_yn_zn { -xp_yp_zp };

		constexpr auto xn { xn_yn_zn.x };
		constexpr auto yn { xn_yn_zn.y };
		constexpr auto zn { xn_yn_zn.z };

		constexpr auto xp { xp_yp_zp.x };
		constexpr auto yp { xp_yp_zp.y };
		constexpr auto zp { xp_yp_zp.z };

		//Top
		constexpr glm::vec3 xn_yp_zp { xn, yp, zp }; // (- + +)
		constexpr glm::vec3 xn_yp_zn { xn, yp, zn }; // (- + -)
		constexpr glm::vec3 xp_yp_zn { xp, yp, zn }; // (+ + -)

		/*
		 * Top-Down view (X,Y,Z)
		 * (-,+,+) =========== (+,+,+)
		 * ||					||
		 * ||					||
		 * (-,+,-) =========== (+,+,-)
		 *
		 * Order (Top)
		 * 1 =========== 0
		 * ||			||
		 * ||			||
		 * 2 =========== 3
		 *
		 * Order (Bottom)
		 * 5 =========== 4
		 * ||			||
		 * ||			||
		 * 6 =========== 7
		 */

		// points[ 0 ] = Coordinate< CType >( xp_yp_zp );
		// points[ 1 ] = Coordinate< CType >( xn_yp_zp );
		// points[ 2 ] = Coordinate< CType >( xn_yp_zn );
		// points[ 3 ] = Coordinate< CType >( xp_yp_zn );

		//Bottom
		constexpr glm::vec3 xn_yn_zp { xn, yn, zp }; // (- - +)
		constexpr glm::vec3 xp_yn_zn { xp, yn, zn }; // (+ - -)
		constexpr glm::vec3 xp_yn_zp { xp, yn, zp }; // (+ - +)

		/*
		 * Bottom-Top view (X,Y,Z)
		 * (-,-,+) =========== (+,-,+)
		 * ||					||
		 * ||					||
		 * (-,-,-) =========== (+,-,-)
		 */

		// points[ 4 ] = Coordinate< CType >( xp_yn_zp );
		// points[ 5 ] = Coordinate< CType >( xn_yn_zp );
		// points[ 6 ] = Coordinate< CType >( xn_yn_zn );
		// points[ 7 ] = Coordinate< CType >( xp_yn_zn );

		constexpr static std::array< Coordinate< CType >, POINT_COUNT > const_points {
			Coordinate< CType >( xp_yp_zp ), Coordinate< CType >( xn_yp_zp ), Coordinate< CType >( xn_yp_zn ),
			Coordinate< CType >( xp_yp_zn ), Coordinate< CType >( xp_yn_zp ), Coordinate< CType >( xn_yn_zp ),
			Coordinate< CType >( xn_yn_zn ), Coordinate< CType >( xp_yn_zn )
		};

		std::array< Coordinate< CType >, POINT_COUNT > points {};

		static_assert( const_points.size() == points.size() );

		for ( auto i = 0; i < const_points.size(); ++i )
		{
			FGL_ASSUME( i < POINT_COUNT + 1 );
			const glm::mat4 rot_mat { m_transform.mat4() };
			const glm::vec3 point { const_points[ i ].vec() };

			points[ i ] = Coordinate< CType >( rot_mat * glm::vec4( point, 1.0f ) );
		}

		return points;
	}

	template < CoordinateSpace CType >
	OrientedBoundingBox< CType > OrientedBoundingBox< CType >::combine( const OrientedBoundingBox< CType >& other )
		const
	{
		ZoneScoped;
		assert( m_transform.translation.vec() != constants::DEFAULT_VEC3 );
		assert( m_transform.scale != glm::vec3( 0.0f ) );

		assert( other.m_transform.translation.vec() != constants::DEFAULT_VEC3 );
		assert( other.m_transform.scale != glm::vec3( 0.0f ) );

		const auto& other_points { other.points() };
		const auto points { this->points() };
		std::array< Coordinate< CType >, interface::BoundingBox::POINT_COUNT * 2 > combined_points {};
		std::copy( other_points.begin(), other_points.end(), combined_points.begin() );
		std::copy( points.begin(), points.end(), combined_points.begin() + POINT_COUNT );

		//TODO: There might be a way to do this without needing to do yet another point calculation.
		return generateBoundingFromPoints< CType >( combined_points );
	}

	template < CoordinateSpace CType >
	AxisAlignedBoundingBox< CType > OrientedBoundingBox< CType >::alignToWorld()
	{
		const auto points { this->points() };
		glm::vec3 max { std::numeric_limits< glm::vec3::type >::infinity() };
		glm::vec3 min { -std::numeric_limits< glm::vec3::type >::infinity() };

		for ( const auto& point : points )
		{
			max.x = glm::max( max.x, point.x );
			max.y = glm::max( max.y, point.y );
			max.z = glm::max( max.z, point.z );

			min.x = glm::min( min.x, point.x );
			min.y = glm::min( min.y, point.y );
			min.z = glm::min( min.z, point.z );
		}

		return AxisAlignedBoundingBox< CType >( Coordinate< CType >( max ), Coordinate< CType >( min ) );
	}

	template < CoordinateSpace CType >
	std::array< LineSegment< CType >, interface::BoundingBox::LINE_COUNT > OrientedBoundingBox< CType >::lines() const
	{
		const auto points { this->points() };

		std::array< LineSegment< CType >, LINE_COUNT > lines;

		//Top
		lines[ 0 ] = LineSegment< CType >( points[ 0 ], points[ 1 ] );
		lines[ 1 ] = LineSegment< CType >( points[ 1 ], points[ 2 ] );
		lines[ 2 ] = LineSegment< CType >( points[ 2 ], points[ 3 ] );
		lines[ 3 ] = LineSegment< CType >( points[ 3 ], points[ 0 ] );

		//Bottom
		lines[ 4 ] = LineSegment< CType >( points[ 4 ], points[ 5 ] );
		lines[ 5 ] = LineSegment< CType >( points[ 5 ], points[ 6 ] );
		lines[ 6 ] = LineSegment< CType >( points[ 6 ], points[ 7 ] );
		lines[ 7 ] = LineSegment< CType >( points[ 7 ], points[ 4 ] );

		//Sides
		lines[ 8 ] = LineSegment< CType >( points[ 0 ], points[ 4 ] );
		lines[ 9 ] = LineSegment< CType >( points[ 1 ], points[ 5 ] );
		lines[ 10 ] = LineSegment< CType >( points[ 2 ], points[ 6 ] );
		lines[ 11 ] = LineSegment< CType >( points[ 3 ], points[ 7 ] );

		return lines;
	}

	template < CoordinateSpace CType, std::size_t TCount >
	OrientedBoundingBox< CType > generateBoundingFromPoints( const std::array< Coordinate< CType >, TCount >& points )
	{
		ZoneScoped;

		constexpr auto inf_float { std::numeric_limits< float >::infinity() };

		// neg (min)
		glm::vec3 top_right_forward { -inf_float };
		// pos (max)
		glm::vec3 bottom_left_back { inf_float };

		assert( points.size() > 0 );

		for ( const auto& pos : points )
		{
			const auto vec { pos.vec() };
			top_right_forward.x = std::max( vec.x, top_right_forward.x );
			top_right_forward.y = std::max( vec.y, top_right_forward.y );
			top_right_forward.z = std::max( vec.z, top_right_forward.z );

			bottom_left_back.x = std::min( vec.x, bottom_left_back.x );
			bottom_left_back.y = std::min( vec.y, bottom_left_back.y );
			bottom_left_back.z = std::min( vec.z, bottom_left_back.z );
		}

		assert( top_right_forward.x != -inf_float );
		assert( top_right_forward.y != -inf_float );
		assert( top_right_forward.z != -inf_float );

		assert( bottom_left_back.x != inf_float );
		assert( bottom_left_back.y != inf_float );
		assert( bottom_left_back.z != inf_float );

		//Calculate midpoint
		const glm::vec3 midpoint { ( top_right_forward + bottom_left_back ) / glm::vec3( 2.0f ) };
		const glm::vec3 scale { glm::abs( bottom_left_back - midpoint ) };

		return { Coordinate< CType >( midpoint ), scale };
	}

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< ModelVertex >& verts )
	{
		assert( verts.size() > 0 );
		log::debug( "Generating bounding box for {} verts", verts.size() );

		constexpr auto inf_float { std::numeric_limits< float >::infinity() };
		// neg (min)
		glm::vec3 top_right_forward { -inf_float };
		// pos (max)
		glm::vec3 bottom_left_back { inf_float };

		for ( const auto& vert : verts )
		{
			const auto& pos { vert.m_position };
			top_right_forward.x = std::max( pos.x, top_right_forward.x );
			top_right_forward.y = std::max( pos.y, top_right_forward.y );
			top_right_forward.z = std::max( pos.z, top_right_forward.z );

			bottom_left_back.x = std::min( pos.x, bottom_left_back.x );
			bottom_left_back.y = std::min( pos.y, bottom_left_back.y );
			bottom_left_back.z = std::min( pos.z, bottom_left_back.z );
		}

		assert( top_right_forward.x != -inf_float );
		assert( top_right_forward.y != -inf_float );
		assert( top_right_forward.z != -inf_float );

		assert( bottom_left_back.x != inf_float );
		assert( bottom_left_back.y != inf_float );
		assert( bottom_left_back.z != inf_float );

		//Calculate midpoint
		const glm::vec3 midpoint { ( top_right_forward + bottom_left_back ) / glm::vec3( 2.0f ) };
		const glm::vec3 scale { glm::abs( bottom_left_back - midpoint ) };

		return OrientedBoundingBox< CoordinateSpace::Model >( Coordinate< CoordinateSpace::Model >( midpoint ), scale );
	}

	//Synthesize the template
	template struct OrientedBoundingBox< CoordinateSpace::Model >;
	template struct OrientedBoundingBox< CoordinateSpace::World >;

} // namespace fgl::engine
