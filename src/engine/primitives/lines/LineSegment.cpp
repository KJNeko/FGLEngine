//
// Created by kj16609 on 2/29/24.
//

#include "LineSegment.hpp"

namespace fgl::engine
{

	template class LineSegment< CoordinateSpace::Model >;
	template class LineSegment< CoordinateSpace::World >;

} // namespace fgl::engine
