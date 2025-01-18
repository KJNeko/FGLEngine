//
// Created by kj16609 on 10/28/24.
//

#include "raycast.hpp"

#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{
	Ray constructRay( const WorldCoordinate& start, const WorldCoordinate& end )
	{
		const Ray ray { start, start.vectorTo( end ).normalize() };
		return ray;
	}

	bool rayHit( const Ray& ray, const OrientedBoundingBox< CS::World >& obb )
	{
		FGL_TODO();
		return false;
	}

	bool rayHit( const Ray& ray, const AxisAlignedBoundingBox< CS::World >& aabb )
	{
		FGL_TODO();
	}

} // namespace fgl::engine