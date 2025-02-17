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
		// Since the point will be in the same space as the oriented box, we can invert the transform of the bounding box's points and move the point,
		// we can then test it using the AABB way

		const glm::mat4 matrix { obb.getMatrix() };
		const glm::mat4 inverse { glm::inverse( matrix ) };

		const glm::vec3 transformed { inverse * glm::vec4( point.vec(), 1.0f ) };

		// this transformed point will now be in the space of the bounding box. So we can now just test it like an AABB
		const bool top_in_range { glm::all( glm::lessThanEqual( transformed, obb.topRightForward().vec() ) ) };
		const bool bottom_in_range { glm::all( glm::greaterThanEqual( transformed, obb.bottomLeftBack().vec() ) ) };

		return top_in_range && bottom_in_range;
	}

	template bool contains< CS::World >( const OrientedBoundingBox< CS::World >&, const Coordinate< CS::World >& );

} // namespace fgl::engine::intersections
