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
		const Coordinate< CType > centered_coordinate { coordinate - m_middle };

		return ( ( centered_coordinate.template x ) < m_span && ( centered_coordinate.template x ) > -m_span )
		    && ( ( centered_coordinate.template y ) < m_span && ( centered_coordinate.template y ) > -m_span )
		    && ( ( centered_coordinate.template z ) < m_span && ( centered_coordinate.template z ) > -m_span );
	}

	//template class AxisAlignedBoundingCube< CoordinateSpace::Model >;
	template class AxisAlignedBoundingCube< CoordinateSpace::World >;

} // namespace fgl::engine
