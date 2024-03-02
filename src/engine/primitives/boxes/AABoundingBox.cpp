//
// Created by kj16609 on 2/29/24.
//

#include "AABoundingBox.hpp"

namespace fgl::engine
{
	template class AxisAlignedBoundingBox< CoordinateSpace::Model >;
	template class AxisAlignedBoundingBox< CoordinateSpace::World >;
} // namespace fgl::engine
