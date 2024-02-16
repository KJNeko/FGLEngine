//
// Created by kj16609 on 2/15/24.
//

#include <catch2/catch_all.hpp>

#define EXPOSE_CAMERA_INTERNAL
#include "engine/Camera.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

TEST_CASE( "Camera", "[camera]" )
{
	Camera camera;

	SECTION( "Perspective set" )
	{
		camera.setPerspectiveProjection( 90.0f, 1.0f, constants::NEAR_PLANE, constants::FAR_PLANE );
	}

	SECTION( "Orthographic set" )
	{
		camera.setOrthographicProjection( 1.0f, 1.0f, 1.0f, 1.0f, constants::NEAR_PLANE, constants::FAR_PLANE );
	}

	const auto camera_up { camera.getUp() };
	REQUIRE( camera_up == constants::WORLD_UP );

	const auto camera_forward { camera.getForward() };
	REQUIRE( camera_forward == constants::WORLD_FORWARD );

	const auto camera_right { camera.getRight() };
	REQUIRE( camera_right == constants::WORLD_RIGHT );
}