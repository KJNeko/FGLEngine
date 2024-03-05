//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include "BoundingBox.hpp"
#include "engine/math/midpoint.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{
	//! Bounding box alligned with the world axis
	template < CoordinateSpace CType >
	class AxisAlignedBoundingBox : public interface::BoundingBox
	{
		Coordinate< CType > m_top_right_forward;
		Coordinate< CType > m_bottom_left_back;

	  public:

		constexpr static auto SpaceType { CType };

		AxisAlignedBoundingBox() :
		  m_top_right_forward( constants::WORLD_CENTER ),
		  m_bottom_left_back( constants::WORLD_CENTER )
		{}

		explicit AxisAlignedBoundingBox(
			const Coordinate< CType > top_right_forward, const Coordinate< CType > bottom_left_back ) :
		  m_top_right_forward( top_right_forward ),
		  m_bottom_left_back( bottom_left_back )
		{}

		explicit AxisAlignedBoundingBox( const Coordinate< CType > midpoint, const Scale scale ) :
		  AxisAlignedBoundingBox( midpoint + scale, midpoint - scale )
		{}

		virtual Coordinate< CType > getPosition() const
		{
			return fgl::midpoint( m_top_right_forward, m_bottom_left_back );
		}

		virtual inline Coordinate< CType > topLeftForward() const { return m_top_right_forward; }

		virtual inline Coordinate< CType > bottomLeftBack() const { return m_bottom_left_back; }

		virtual Scale scale() const;

		std::vector< Coordinate< CType > > points() const;
		std::vector< LineSegment< CType > > lines() const;

		constexpr NormalVector right() const { return NormalVector::bypass( constants::WORLD_RIGHT ); }

		constexpr NormalVector up() const { return NormalVector::bypass( constants::WORLD_UP ); }

		constexpr NormalVector forward() const { return NormalVector::bypass( constants::WORLD_FORWARD ); }
	};

} // namespace fgl::engine