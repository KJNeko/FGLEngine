//
// Created by kj16609 on 10/31/24.
//

#include "engine/primitives/points/Coordinate.hpp"

#include "engine/primitives/boxes/AxisAlignedBoundingBox.hpp"

namespace fgl::engine::intersections
{

	template < CS CType >
	bool contains( const AxisAlignedBoundingBox< CType >& aabb, const Coordinate< CType >& point )
	{
		const Coordinate< CType > centered_coordinate { point - aabb.getPosition() };

		const bool top_in_range { glm::all( glm::lessThanEqual( centered_coordinate.vec(), aabb.scale() ) ) };
		const bool bottom_in_range { glm::all( glm::greaterThanEqual( centered_coordinate.vec(), -aabb.scale() ) ) };

		return top_in_range && bottom_in_range;
	}



	template bool contains< CS::World >( const AxisAlignedBoundingBox< CS::World >&, const Coordinate< CS::World >& );

} // namespace fgl::engine::intersections
