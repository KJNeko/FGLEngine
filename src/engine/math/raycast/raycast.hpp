//
// Created by kj16609 on 10/28/24.
//

#pragma once
#include "engine/primitives/Rotation.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	class OctTreeNode;

	struct Ray
	{
		WorldCoordinate start;
		NormalVector vector;
	};

	Ray constructRay( WorldCoordinate& start, WorldCoordinate& end );

	bool rayHit( const Ray& ray, const OrientedBoundingBox< CS::World >& obb, OctTreeNode* root );

	bool rayHit( const Ray& ray, const AxisAlignedBoundingBox< CS::World >& aabb, OctTreeNode* root );

} // namespace fgl::engine
