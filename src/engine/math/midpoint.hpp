//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include "engine/primitives/CoordinateSpace.hpp"

namespace fgl
{
	namespace engine
	{
		template < CoordinateSpace >
		class Coordinate;
	}

	template < engine::CoordinateSpace CType >
	engine::Coordinate< CType >
		midpoint( const engine::Coordinate< CType > left, const engine::Coordinate< CType > right );

} // namespace fgl
