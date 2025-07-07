//
// Created by kj16609 on 10/31/24.
//

#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"

namespace fgl::engine::intersections
{

	template < CS CType >
	bool contains( const AxisAlignedBoundingCube< CType >& aabb, const Coordinate< CType >& point );

} // namespace fgl::engine::intersections
