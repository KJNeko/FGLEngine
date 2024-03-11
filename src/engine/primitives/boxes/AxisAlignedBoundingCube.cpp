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

		return ( ( centered_coordinate.template x ) < this->span()
		         && ( centered_coordinate.template x ) > -this->span() )
		    && ( ( centered_coordinate.template y ) < this->span()
		         && ( centered_coordinate.template y ) > -this->span() )
		    && ( ( centered_coordinate.template z ) < this->span()
		         && ( centered_coordinate.template z ) > -this->span() );
	}

	//template class AxisAlignedBoundingCube< CoordinateSpace::Model >;
	template class AxisAlignedBoundingCube< CoordinateSpace::World >;

} // namespace fgl::engine
