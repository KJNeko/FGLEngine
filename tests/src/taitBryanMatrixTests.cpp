//
// Created by kj16609 on 2/19/24.
//

#include <catch2/catch_all.hpp>

#include "engine/Camera.hpp"
#include "engine/math/taitBryanMatrix.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

TEST_CASE( "Tait-Bryan rotations", "[matrix][rotation][camera]" )
{
	WHEN( "Rotation is default constructed" )
	{
		THEN( "The rotation is (0,0,0)" )
		{
			Rotation rotation;
			REQUIRE( rotation.pitch() == 0.0f );
			REQUIRE( rotation.yaw() == 0.0f );
			REQUIRE( rotation.roll() == 0.0f );
		}
	}

	WHEN( "Given a default rotation (0,0,0)" )
	{
		Rotation rot;

		THEN( "The rotation matrix is the identity matrix" )
		{
			REQUIRE( constants::MAT4_IDENTITY == taitBryanMatrix( rot ) );
		}
	}
}
