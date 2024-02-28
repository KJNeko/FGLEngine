//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include "engine/primitives/points/points.hpp"
#include "engine/primitives/vectors/vectors.hpp"

namespace fgl::engine
{

	template < typename T >
	concept is_line = requires( T t ) {
		{
			t.getPosition()
		} -> is_coordinate;
		{
			t.getDirection()
		} -> is_normal_vector;
	};

} // namespace fgl::engine