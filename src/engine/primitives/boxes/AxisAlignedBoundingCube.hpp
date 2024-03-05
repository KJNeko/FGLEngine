//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include "AxisAlignedBoundingBox.hpp"
#include "BoundingBox.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	class AxisAlignedBoundingCube final : public AxisAlignedBoundingBox< CType >
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

		bool contains( const Coordinate< CType >& coordinate ) const;

		float span() const { return m_span; }

		Scale scale() const override { return Scale( m_span, m_span, m_span ); }

		inline Coordinate< CType > topLeftForward() const override { return m_middle + scale(); }

		inline Coordinate< CType > bottomLeftBack() const override { return m_middle - scale(); }

		Coordinate< CType > getPosition() const override { return m_middle; }

		constexpr NormalVector right() const { return NormalVector::bypass( constants::WORLD_RIGHT ); }

		constexpr NormalVector up() const { return NormalVector::bypass( constants::WORLD_UP ); }

		constexpr NormalVector forward() const { return NormalVector::bypass( constants::WORLD_FORWARD ); }
	};

} // namespace fgl::engine
