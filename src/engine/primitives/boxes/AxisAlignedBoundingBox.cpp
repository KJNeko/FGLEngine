//
// Created by kj16609 on 2/29/24.
//

#include "AxisAlignedBoundingBox.hpp"

#include "OrientedBoundingBox.hpp"
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

	template < CoordinateSpace CType >
	AxisAlignedBoundingBox< CType >& AxisAlignedBoundingBox< CType >::combine( const AxisAlignedBoundingBox& other )
	{
		const Coordinate< CType > new_top_right_forward {
			std::max( this->m_top_right_forward.x, other.m_top_right_forward.x ),
			std::max( this->m_top_right_forward.y, other.m_top_right_forward.y ),
			std::max( this->m_top_right_forward.z, other.m_top_right_forward.z ),
		};

		const Coordinate< CType > new_bottom_left_back {
			std::min( this->m_bottom_left_back.x, other.m_bottom_left_back.x ),
			std::min( this->m_bottom_left_back.y, other.m_bottom_left_back.y ),
			std::min( this->m_bottom_left_back.z, other.m_bottom_left_back.z ),
		};

		this->m_top_right_forward = new_top_right_forward;
		this->m_bottom_left_back = new_bottom_left_back;

		return *this;
	}

	template < CoordinateSpace CType >
	AxisAlignedBoundingBox< CType >::AxisAlignedBoundingBox( const OrientedBoundingBox< CType >& oobb ) :
	  m_top_right_forward( -constants::DEFAULT_VEC3 ),
	  m_bottom_left_back( constants::DEFAULT_VEC3 )
	{
		if ( oobb.rotation == Rotation() ) // If default rotation then we can simply just take it as the box is
		{
			m_top_right_forward = oobb.middle + oobb.scale;
			m_bottom_left_back = oobb.middle - oobb.scale;
		}
		else
		{
			//Rotation has been done. Need to use the points to figure it out
			for ( const auto& point : oobb.points() )
			{
				m_top_right_forward.x = std::max( m_top_right_forward.x, point.x );
				m_top_right_forward.y = std::max( m_top_right_forward.y, point.y );
				m_top_right_forward.z = std::max( m_top_right_forward.z, point.z );

				m_bottom_left_back.x = std::min( m_bottom_left_back.x, point.x );
				m_bottom_left_back.y = std::min( m_bottom_left_back.y, point.y );
				m_bottom_left_back.z = std::min( m_bottom_left_back.z, point.z );
			}
		}
	}

	template < CoordinateSpace CType >
	AxisAlignedBoundingBox< CType >& AxisAlignedBoundingBox< CType >::combine( const OrientedBoundingBox< CType >&
	                                                                               other )
	{
		AxisAlignedBoundingBox< CType > aabb { other };
		if ( this->m_top_right_forward == Coordinate< CType >( constants::DEFAULT_VEC3 )
		     || this->m_bottom_left_back == Coordinate< CType >( constants::DEFAULT_VEC3 ) )
			return *this = aabb;
		else
		{
			return this->combine( aabb );
		}
	}

	template class AxisAlignedBoundingBox< CoordinateSpace::Model >;
	template class AxisAlignedBoundingBox< CoordinateSpace::World >;
} // namespace fgl::engine
