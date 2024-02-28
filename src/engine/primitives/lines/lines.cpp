//
// Created by kj16609 on 2/28/24.
//

#include "lines.hpp"

#include "InfiniteLine.hpp"
#include "LineSegment.hpp"

using namespace fgl::engine;

static_assert( is_line< InfiniteLine< CoordinateSpace::Model > > );
static_assert( is_line< LineSegment< CoordinateSpace::Model > > );
