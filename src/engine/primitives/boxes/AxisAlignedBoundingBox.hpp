//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include "BoundingBox.hpp"
#include "engine/FGL_DEFINES.hpp"
#include "engine/primitives/Scale.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct OrientedBoundingBox;

	//! Bounding box alligned with the world axis
	template < CoordinateSpace CType >
	class AxisAlignedBoundingBox : public interface::BoundingBox
	{
		Coordinate< CType > m_top_right_forward;
		Coordinate< CType > m_bottom_left_back;

	  public:

		constexpr static auto SpaceType { CType };

		AxisAlignedBoundingBox(
			const Coordinate< CType > top_right_forward, const Coordinate< CType > bottom_left_back ) :
		  m_top_right_forward( top_right_forward ),
		  m_bottom_left_back( bottom_left_back )
		{
			assert( m_top_right_forward.x > m_bottom_left_back.x );
			assert( m_top_right_forward.y > m_bottom_left_back.y );
			assert( m_top_right_forward.z > m_bottom_left_back.z );

			FGL_NOTNAN( m_bottom_left_back.x );
			FGL_NOTNAN( m_bottom_left_back.y );
			FGL_NOTNAN( m_bottom_left_back.z );
			FGL_NOTNAN( m_bottom_left_back.x );
			FGL_NOTNAN( m_bottom_left_back.y );
			FGL_NOTNAN( m_bottom_left_back.z );
		}

		AxisAlignedBoundingBox( const Coordinate< CType > midpoint, const Scale scale ) :
		  AxisAlignedBoundingBox( midpoint + scale, midpoint - scale )
		{
			assert( m_top_right_forward.x > m_bottom_left_back.x );
			assert( m_top_right_forward.y > m_bottom_left_back.y );
			assert( m_top_right_forward.z > m_bottom_left_back.z );

			FGL_NOTNAN( m_bottom_left_back.x );
			FGL_NOTNAN( m_bottom_left_back.y );
			FGL_NOTNAN( m_bottom_left_back.z );
			FGL_NOTNAN( m_bottom_left_back.x );
			FGL_NOTNAN( m_bottom_left_back.y );
			FGL_NOTNAN( m_bottom_left_back.z );
		}

		//TODO: This should be removed from access in favor of OBB::alignToWorld()
		explicit AxisAlignedBoundingBox( const OrientedBoundingBox< CType >& oobb );

		AxisAlignedBoundingBox& combine( const AxisAlignedBoundingBox& other );
		AxisAlignedBoundingBox& combine( const OrientedBoundingBox< CType >& other );

		//AxisAlignedBoundingBox& combine( const OrientedBoundingBox< CType >& other );

		bool operator==( const AxisAlignedBoundingBox< CType >& other ) const
		{
			return m_top_right_forward == other.m_top_right_forward && m_bottom_left_back == other.m_bottom_left_back;
		}

		virtual Coordinate< CType > getPosition() const final
		{
			return midpoint( m_top_right_forward, m_bottom_left_back );
		}

		virtual inline Coordinate< CType > topLeftForward() const final { return m_top_right_forward; }

		virtual inline Coordinate< CType > bottomLeftBack() const final { return m_bottom_left_back; }

		virtual Scale scale() const;

		std::array< Coordinate< CType >, POINT_COUNT > points() const;
		std::array< LineSegment< CType >, LINE_COUNT > lines() const;

		constexpr NormalVector right() const { return NormalVector( constants::WORLD_X ); }

		constexpr NormalVector up() const { return NormalVector( constants::WORLD_Z ); }

		constexpr NormalVector forward() const { return NormalVector( constants::WORLD_Y ); }
	};

} // namespace fgl::engine