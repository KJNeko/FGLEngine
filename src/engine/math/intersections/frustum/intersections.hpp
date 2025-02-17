//
// Created by kj16609 on 10/28/24.
//

#pragma once
#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"

namespace fgl::engine
{
	struct Frustum;
}

namespace fgl::engine::intersections
{

	bool intersects( const Frustum& frustum, const AxisAlignedBoundingBox< CS::World >& aabb );
	bool intersects( const Frustum& frustum, const AxisAlignedBoundingCube< CS::World >& aabc );
	bool intersects( const Frustum& frustum, const OrientedBoundingBox< CS::World >& obb );

} // namespace fgl::engine::intersections