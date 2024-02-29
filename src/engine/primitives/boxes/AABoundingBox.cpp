//
// Created by kj16609 on 2/29/24.
//

#include "AABoundingBox.hpp"

#include "engine/math/midpoint.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	Coordinate< CType > AxisAlignedBoundingBox< CType >::center() const
	{
		return fgl::midpoint( m_top_right_forward, m_bottom_left_back );
	}

	template class AxisAlignedBoundingBox< CoordinateSpace::Model >;
	template class AxisAlignedBoundingBox< CoordinateSpace::World >;

} // namespace fgl::engine
