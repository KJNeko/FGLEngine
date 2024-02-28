//
// Created by kj16609 on 1/27/24.
//

#include "OrientedBoundingBox.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <array>

#include "engine/model/Vertex.hpp"
#include "engine/primitives/Coordinate.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Matrix.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	bool OrientedBoundingBox< CType >::isInFrustum( const Frustum< CType >& frustum ) const
	{
		if constexpr ( CType != CoordinateSpace::World )
		{
			//TODO: Figure out how to make this a compiler error
			throw std::runtime_error( "Frustum and bounding box must be in World coordinate space!" );
		}
		else
		{
			ZoneScoped;
			const std::vector< Coordinate< CType > >& points { this->points() };
			for ( const auto& point : points )
			{
				if ( frustum.pointInside( point ) ) return true;
			}

			return false;
		}
	}

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
	std::vector< Coordinate< CType > > OrientedBoundingBox< CType >::points() const
	{
		assert( middle != constants::DEFAULT_VEC3 );
		assert( scale != glm::vec3( 0.0f ) );
		std::vector< Coordinate< CType > > points;

		// xp == x positive (Highest x point)
		// xn == x negative (Lowest x point)

		const glm::vec3 xp_yp_zp { middle + scale };
		const glm::vec3 xn_yn_zn { middle - scale };

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

		points.resize( 8 );
		points[ 0 ] = xp_yp_zp;
		points[ 1 ] = xn_yp_zp;
		points[ 2 ] = xn_yp_zn;
		points[ 3 ] = xp_yp_zn;

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

		points[ 4 ] = xp_yn_zp;
		points[ 5 ] = xn_yn_zp;
		points[ 6 ] = xn_yn_zn;
		points[ 7 ] = xp_yn_zn;

		return points;
	}

	template < CoordinateSpace CType >
	OrientedBoundingBox< CType > OrientedBoundingBox< CType >::combine( const OrientedBoundingBox< CType >& other )
		const
	{
		assert( middle != constants::DEFAULT_VEC3 );
		assert( scale != glm::vec3( 0.0f ) );

		assert( other.middle != constants::DEFAULT_VEC3 );
		assert( other.scale != glm::vec3( 0.0f ) );

		ZoneScoped;
		const auto& other_points { other.points() };
		std::vector< Coordinate< CType > > points { this->points() };
		points.insert( points.end(), other_points.begin(), other_points.end() );

		//TODO: There might be a way to do this without needing to do yet another point calculation.
		return generateBoundingFromPoints< CType >( points );
	}

	template < CoordinateSpace CType >
	std::vector< Line< CType > > OrientedBoundingBox< CType >::lines() const
	{
		const auto points { this->points() };

		std::vector< Line< CType > > lines;

		//Top
		lines.emplace_back( points[ 0 ], points[ 1 ] );
		lines.emplace_back( points[ 1 ], points[ 2 ] );
		lines.emplace_back( points[ 2 ], points[ 3 ] );
		lines.emplace_back( points[ 3 ], points[ 0 ] );

		//Bottom
		lines.emplace_back( points[ 4 ], points[ 5 ] );
		lines.emplace_back( points[ 5 ], points[ 6 ] );
		lines.emplace_back( points[ 6 ], points[ 7 ] );
		lines.emplace_back( points[ 7 ], points[ 4 ] );

		//Sides
		lines.emplace_back( points[ 0 ], points[ 4 ] );
		lines.emplace_back( points[ 1 ], points[ 5 ] );
		lines.emplace_back( points[ 2 ], points[ 6 ] );
		lines.emplace_back( points[ 3 ], points[ 7 ] );

		return lines;
	}

	template < CoordinateSpace CType >
	OrientedBoundingBox< CType > generateBoundingFromPoints( const std::vector< Coordinate< CType > >& points )
	{
		ZoneScoped;
		assert( points.size() > 0 );

		// neg (min)
		glm::vec3 top_left_front { points[ 0 ] };
		// pos (max)
		glm::vec3 bottom_right_back { points[ 0 ] };

		for ( const auto& pos : points )
		{
			top_left_front.x = std::min( static_cast< glm::vec3 >( pos ).x, top_left_front.x );
			top_left_front.y = std::min( static_cast< glm::vec3 >( pos ).y, top_left_front.y );
			top_left_front.z = std::min( static_cast< glm::vec3 >( pos ).z, top_left_front.z );

			bottom_right_back.x = std::max( static_cast< glm::vec3 >( pos ).x, bottom_right_back.x );
			bottom_right_back.y = std::max( static_cast< glm::vec3 >( pos ).y, bottom_right_back.y );
			bottom_right_back.z = std::max( static_cast< glm::vec3 >( pos ).z, bottom_right_back.z );
		}

		//Calculate midpoint
		const glm::vec3 midpoint { ( top_left_front + bottom_right_back ) / glm::vec3( 2.0f ) };
		const glm::vec3 scale { bottom_right_back - midpoint };

		return { Coordinate< CType >( midpoint ), scale };
	}

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< Vertex >& verts )
	{
		// neg (min)
		glm::vec3 top_left_front { verts[ 0 ].m_position };
		// pos (max)
		glm::vec3 bottom_right_back { verts[ 0 ].m_position };

		for ( const auto& vert : verts )
		{
			const auto& pos { vert.m_position };
			top_left_front.x = std::min( static_cast< glm::vec3 >( pos ).x, top_left_front.x );
			top_left_front.y = std::min( static_cast< glm::vec3 >( pos ).y, top_left_front.y );
			top_left_front.z = std::min( static_cast< glm::vec3 >( pos ).z, top_left_front.z );

			bottom_right_back.x = std::max( static_cast< glm::vec3 >( pos ).x, bottom_right_back.x );
			bottom_right_back.y = std::max( static_cast< glm::vec3 >( pos ).y, bottom_right_back.y );
			bottom_right_back.z = std::max( static_cast< glm::vec3 >( pos ).z, bottom_right_back.z );
		}

		//Calculate midpoint
		const glm::vec3 midpoint { ( top_left_front + bottom_right_back ) / glm::vec3( 2.0f ) };
		const glm::vec3 scale { bottom_right_back - midpoint };

		return { Coordinate< CoordinateSpace::Model >( midpoint ), scale };
	}

	//Synthesize the template
	template struct OrientedBoundingBox< CoordinateSpace::Model >;
	template struct OrientedBoundingBox< CoordinateSpace::World >;

} // namespace fgl::engine
