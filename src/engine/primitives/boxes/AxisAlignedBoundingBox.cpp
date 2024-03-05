//
// Created by kj16609 on 2/29/24.
//

#include "AxisAlignedBoundingBox.hpp"

#include "engine/primitives/lines/LineSegment.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	Scale AxisAlignedBoundingBox< CType >::scale() const
	{
		return Scale( m_top_right_forward.vec() - getPosition().vec() );
	}

	template < CoordinateSpace CType >
	std::vector< Coordinate< CType > > AxisAlignedBoundingBox< CType >::points() const
	{
		std::vector< Coordinate< CType > > points;

		// xp == x positive (Highest x point)
		// xn == x negative (Lowest x point)

		const glm::vec3 xp_yp_zp { topLeftForward().vec() };
		const glm::vec3 xn_yn_zn { bottomLeftBack().vec() };

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

		points.resize( 8 );
		points[ 0 ] = Coordinate< CType >( xp_yp_zp );
		points[ 1 ] = Coordinate< CType >( xn_yp_zp );
		points[ 2 ] = Coordinate< CType >( xn_yp_zn );
		points[ 3 ] = Coordinate< CType >( xp_yp_zn );

		//Bottom
		const glm::vec3 xn_yn_zp { xn, yn, zp }; // (- - +)
		const glm::vec3 xp_yn_zn { xp, yn, zn }; // (+ - -)
		const glm::vec3 xp_yn_zp { xp, yn, zp }; // (+ - +)

		points[ 4 ] = Coordinate< CType >( xp_yn_zp );
		points[ 5 ] = Coordinate< CType >( xn_yn_zp );
		points[ 6 ] = Coordinate< CType >( xn_yn_zn );
		points[ 7 ] = Coordinate< CType >( xp_yn_zn );

		//Rotate all the points around middle using the rotation

		return points;
	}

	template < CoordinateSpace CType >
	std::vector< LineSegment< CType > > AxisAlignedBoundingBox< CType >::lines() const
	{
		const auto points { this->points() };

		std::vector< LineSegment< CType > > lines;

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

	template class AxisAlignedBoundingBox< CoordinateSpace::Model >;
	template class AxisAlignedBoundingBox< CoordinateSpace::World >;
} // namespace fgl::engine
