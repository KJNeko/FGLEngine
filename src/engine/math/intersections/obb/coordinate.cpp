//
// Created by kj16609 on 10/31/24.
//

#include "engine/primitives/points/Coordinate.hpp"

#include "engine/primitives/boxes/OrientedBoundingBox.hpp"

namespace fgl::engine::intersections
{

	template < CS CType >
	bool contains( const OrientedBoundingBox< CType >& obb, const Coordinate< CType >& point )
	{
		FGL_UNIMPLEMENTED();
	}

	template bool contains< CS::World >( const OrientedBoundingBox< CS::World >&, const Coordinate< CS::World >& );

} // namespace fgl::engine::intersections
