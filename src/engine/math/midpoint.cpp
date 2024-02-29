//
// Created by kj16609 on 2/29/24.
//

#include "midpoint.hpp"

#include "engine/primitives/points/Coordinate.hpp"

using namespace fgl::engine;

namespace fgl
{

	template < CoordinateSpace CType >
	Coordinate< CType > midpoint( const Coordinate< CType > left, const Coordinate< CType > right )
	{
		const auto x { ( left.x + right.x ) / 2.0f };
		const auto y { ( left.y + right.y ) / 2.0f };
		const auto z { ( left.z + right.z ) / 2.0f };

		return Coordinate< CType >( x, y, z );
	}

	template Coordinate< CoordinateSpace::Model >
		midpoint( const Coordinate< CoordinateSpace::Model >, const Coordinate< CoordinateSpace::Model > );
	template Coordinate< CoordinateSpace::World >
		midpoint( const Coordinate< CoordinateSpace::World >, const Coordinate< CoordinateSpace::World > );

} // namespace fgl
