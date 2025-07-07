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
	  public:

		constexpr static auto SpaceType { CType };

		explicit AxisAlignedBoundingCube( const Coordinate< CType > middle, const float span ) :
		  AxisAlignedBoundingBox< CType >( middle, Scale( span, span, span ) )
		{}

		bool contains( const Coordinate< CType >& coordinate ) const;

		float span() const { return this->scale().x; }

		constexpr NormalVector right() const { return NormalVector( constants::WORLD_X ); }

		constexpr NormalVector up() const { return NormalVector( constants::WORLD_Z ); }

		constexpr NormalVector forward() const { return NormalVector( constants::WORLD_Y ); }

		operator AxisAlignedBoundingBox< CType >() const
		{
			return static_cast< AxisAlignedBoundingBox< CType > >( *this );
		}
	};

} // namespace fgl::engine
