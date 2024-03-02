//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include "BoundingBox.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	class AxisAlignedBoundingCube : public interface::BoundingBox
	{
		Coordinate< CType > m_middle;
		float m_span;

	  public:

		constexpr static auto SpaceType { CType };

		AxisAlignedBoundingCube() : m_middle( constants::WORLD_CENTER ), m_span( 1.0f ) {}

		explicit AxisAlignedBoundingCube( const Coordinate< CType > middle, const float span ) :
		  m_middle( middle ),
		  m_span( span )
		{}

		inline float span() const { return m_span; }

		inline Coordinate< CType > getPosition() const { return m_middle; }

		inline Coordinate< CType > topLeftForward() { return m_middle + Coordinate< CType >( m_span ); }

		inline Coordinate< CType > bottomLeftBack() const { m_middle - Coordinate< CType >( m_span ); }
	};

} // namespace fgl::engine
