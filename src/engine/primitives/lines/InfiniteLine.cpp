//
// Created by kj16609 on 2/29/24.
//

#include "InfiniteLine.hpp"

#include "engine/primitives/planes/OriginDistancePlane.hpp"
#include "engine/primitives/planes/PointPlane.hpp"

namespace fgl::engine
{

	template class InfiniteLine< CoordinateSpace::Model >;
	template class InfiniteLine< CoordinateSpace::World >;

} // namespace fgl::engine
