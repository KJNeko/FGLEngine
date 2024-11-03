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
	std::array< Coordinate< CType >, interface::BoundingBox::POINT_COUNT > AxisAlignedBoundingBox< CType >::points()
		const
	{
		std::array< Coordinate< CType >, POINT_COUNT > points {};

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
	std::array< LineSegment< CType >, interface::BoundingBox::LINE_COUNT > AxisAlignedBoundingBox< CType >::lines()
		const
	{
		const auto points { this->points() };

		std::array< LineSegment< CType >, LINE_COUNT > lines {};

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

	template < CoordinateSpace CType >
	AxisAlignedBoundingBox< CType >& AxisAlignedBoundingBox< CType >::combine( const AxisAlignedBoundingBox& other )
	{
		FGL_NOTNANVEC3( other.m_top_right_forward );
		FGL_NOTNANVEC3( other.m_bottom_left_back );
		FGL_NOTNANVEC3( this->m_top_right_forward );
		FGL_NOTNANVEC3( this->m_bottom_left_back );

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
	AxisAlignedBoundingBox< CType >& AxisAlignedBoundingBox< CType >::combine( const OrientedBoundingBox< CType >&
	                                                                               other )
	{
		const auto other_trf { other.topRightForward() };
		const auto other_blb { other.bottomLeftBack() };

		FGL_NOTNANVEC3( other.topRightForward() );
		FGL_NOTNANVEC3( other.bottomLeftBack() );
		FGL_NOTNANVEC3( this->m_top_right_forward );
		FGL_NOTNANVEC3( this->m_bottom_left_back );

		const Coordinate< CType > new_top_right_forward {
			std::max( this->m_top_right_forward.x, other_trf.x ),
			std::max( this->m_top_right_forward.y, other_trf.y ),
			std::max( this->m_top_right_forward.z, other_trf.z ),
		};

		const Coordinate< CType > new_bottom_left_back {
			std::min( this->m_bottom_left_back.x, other_blb.x ),
			std::min( this->m_bottom_left_back.y, other_blb.y ),
			std::min( this->m_bottom_left_back.z, other_blb.z ),
		};

		this->m_top_right_forward = new_top_right_forward;
		this->m_bottom_left_back = new_bottom_left_back;

		return *this;
	}

	template < CoordinateSpace CType >
	AxisAlignedBoundingBox< CType >::AxisAlignedBoundingBox( const OrientedBoundingBox< CType >& oobb ) :
	  m_top_right_forward( constants::DEFAULT_VEC3 ),
	  m_bottom_left_back( -constants::DEFAULT_VEC3 )
	{
		FGL_NOTNANVEC3( oobb.topRightForward() );
		FGL_NOTNANVEC3( oobb.bottomLeftBack() );
		FGL_NOTNANVEC3( this->m_top_right_forward );
		FGL_NOTNANVEC3( this->m_bottom_left_back );

		if ( oobb.m_transform.rotation
		     == Rotation() ) // If default rotation then we can simply just take it as the box is
		{
			m_top_right_forward = oobb.topRightForward();
			m_bottom_left_back = oobb.bottomLeftBack();
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

	template class AxisAlignedBoundingBox< CoordinateSpace::Model >;
	template class AxisAlignedBoundingBox< CoordinateSpace::World >;
} // namespace fgl::engine
