//
// Created by kj16609 on 2/16/24.
//

#include <catch2/catch_all.hpp>

#include "engine/primitives/Vector.hpp"

using namespace fgl::engine;

TEST_CASE( "Vector", "[vector][transforms]" )
{
	SECTION( "Right after rotation" )
	{
		Vector rotation_vec { constants::WORLD_FORWARD };

		//Rotate by 90 degrees on yaw
	}
}