//
// Created by kj16609 on 10/28/24.
//

#pragma once
#include "engine/primitives/lines/LineSegment.hpp"

namespace fgl::engine
{
	struct Frustum;
}

namespace fgl::engine::intersections
{

	bool intersects( const LineSegment< CS::World >& line, const Frustum& frustum );

}