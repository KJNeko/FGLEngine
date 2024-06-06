//
// Created by kj16609 on 1/27/24.
//

#include "OrientedBoundingBox.hpp"

#include <tracy/Tracy.hpp>

#include <array>

#include "engine/logging/logging.hpp"
#include "engine/model/Vertex.hpp"
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
		assert( middle.vec() != constants::DEFAULT_VEC3 );
		assert( scale != glm::vec3( 0.0f ) );
		std::array< Coordinate< CType >, POINT_COUNT > points {};

		// xp == x positive (Highest x point)
		// xn == x negative (Lowest x point)

		const glm::vec3 xp_yp_zp { middle.vec() + scale };
		const glm::vec3 xn_yn_zn { middle.vec() - scale };

		const auto xn { xn_yn_zn.x };
		const auto yn { xn_yn_zn.y };
		const auto zn { xn_yn_zn.z };

		const auto xp { xp_yp_zp.x };
		const auto yp { xp_yp_zp.y };
		const auto zp { xp_yp_zp.z };

		//Top
		const glm::vec3 xn_yp_zp { xn, yp, zp }; // (- + +)
		const glm::vec3 xn_yp_zn { xn, yp, zn }; // (- + -)
		const glm::vec3 xp_yp_zn { xp, yp, zn }; // (+ + -)

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

		points[ 0 ] = Coordinate< CType >( xp_yp_zp );
		points[ 1 ] = Coordinate< CType >( xn_yp_zp );
		points[ 2 ] = Coordinate< CType >( xn_yp_zn );
		points[ 3 ] = Coordinate< CType >( xp_yp_zn );

		//Bottom
		const glm::vec3 xn_yn_zp { xn, yn, zp }; // (- - +)
		const glm::vec3 xp_yn_zn { xp, yn, zn }; // (+ - -)
		const glm::vec3 xp_yn_zp { xp, yn, zp }; // (+ - +)

		/*
		 * Bottom-Top view (X,Y,Z)
		 * (-,-,+) =========== (+,-,+)
		 * ||					||
		 * ||					||
		 * (-,-,-) =========== (+,-,-)
		 */

		points[ 4 ] = Coordinate< CType >( xp_yn_zp );
		points[ 5 ] = Coordinate< CType >( xn_yn_zp );
		points[ 6 ] = Coordinate< CType >( xn_yn_zn );
		points[ 7 ] = Coordinate< CType >( xp_yn_zn );

		//Rotate all the points around middle using the rotation

		return points;
	}

	template < CoordinateSpace CType >
	OrientedBoundingBox< CType > OrientedBoundingBox< CType >::combine( const OrientedBoundingBox< CType >& other )
		const
	{
		assert( middle.vec() != constants::DEFAULT_VEC3 );
		assert( scale != glm::vec3( 0.0f ) );

		assert( other.middle.vec() != constants::DEFAULT_VEC3 );
		assert( other.scale != glm::vec3( 0.0f ) );

		ZoneScoped;
		const auto& other_points { other.points() };
		const auto points { this->points() };
		std::array< Coordinate< CType >, interface::BoundingBox::POINT_COUNT * 2 > combined_points {};
		std::copy( other_points.begin(), other_points.end(), combined_points.begin() );
		std::copy( points.begin(), points.end(), combined_points.begin() + POINT_COUNT );

		//TODO: There might be a way to do this without needing to do yet another point calculation.
		return generateBoundingFromPoints< CType >( combined_points );
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

		// neg (min)
		glm::vec3 top_right_forward { points[ 0 ].vec() };
		// pos (max)
		glm::vec3 bottom_left_back { points[ 0 ].vec() };

		for ( const auto& pos : points )
		{
			top_right_forward.x = std::max( pos.vec().x, top_right_forward.x );
			top_right_forward.y = std::max( pos.vec().y, top_right_forward.y );
			top_right_forward.z = std::max( pos.vec().z, top_right_forward.z );

			bottom_left_back.x = std::min( pos.vec().x, bottom_left_back.x );
			bottom_left_back.y = std::min( pos.vec().y, bottom_left_back.y );
			bottom_left_back.z = std::min( pos.vec().z, bottom_left_back.z );
		}

		//Calculate midpoint
		const glm::vec3 midpoint { ( top_right_forward + bottom_left_back ) / glm::vec3( 2.0f ) };
		const glm::vec3 scale { bottom_left_back - midpoint };

		return { Coordinate< CType >( midpoint ), scale };
	}

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< Vertex >& verts )
	{
		log::debug( "Generating bounding box for {} verts", verts.size() );
		// neg (min)
		glm::vec3 top_right_forward { verts[ 0 ].m_position };
		// pos (max)
		glm::vec3 bottom_left_back { verts[ 0 ].m_position };

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

		//Calculate midpoint
		const glm::vec3 midpoint { ( top_right_forward + bottom_left_back ) / glm::vec3( 2.0f ) };
		const glm::vec3 scale { bottom_left_back - midpoint };

		return { Coordinate< CoordinateSpace::Model >( midpoint ), scale };
	}

	//Synthesize the template
	template struct OrientedBoundingBox< CoordinateSpace::Model >;
	template struct OrientedBoundingBox< CoordinateSpace::World >;

} // namespace fgl::engine
