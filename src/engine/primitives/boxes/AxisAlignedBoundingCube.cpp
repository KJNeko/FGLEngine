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

		const bool is_high_x { centered_coordinate.x > this->span() };
		const bool is_high_y { centered_coordinate.x > this->span() };
		const bool is_high_z { centered_coordinate.x > this->span() };

		const bool is_low_x { centered_coordinate.x < -this->span() };
		const bool is_low_y { centered_coordinate.x < -this->span() };
		const bool is_low_z { centered_coordinate.x < -this->span() };

		const bool is_high { is_high_x || is_high_y || is_high_z };
		const bool is_low { is_low_x || is_low_y || is_low_z };

		return !is_high && !is_low;
	}

	//template class AxisAlignedBoundingCube< CoordinateSpace::Model >;
	template class AxisAlignedBoundingCube< CoordinateSpace::World >;

} // namespace fgl::engine
