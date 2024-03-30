//
// Created by kj16609 on 3/1/24.
//

#include "AxisAlignedBoundingCube.hpp"

#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	bool AxisAlignedBoundingCube< CType >::contains( const Coordinate< CType >& coordinate ) const
	{
		const Coordinate< CType > centered_coordinate { coordinate - this->getPosition() };

		const bool top_in_range { glm::all( glm::lessThanEqual( centered_coordinate.vec(), this->scale() ) ) };
		const bool bottom_in_range { glm::all( glm::greaterThanEqual( centered_coordinate.vec(), -this->scale() ) ) };

		return top_in_range && bottom_in_range;
	}

	//template class AxisAlignedBoundingCube< CoordinateSpace::Model >;
	template class AxisAlignedBoundingCube< CoordinateSpace::World >;

} // namespace fgl::engine
