//
// Created by kj16609 on 2/15/24.
//

#include <catch2/catch_all.hpp>

#include <iostream>

#define EXPOSE_CAMERA_INTERNAL
#include "engine/Camera.hpp"
#include "engine/model/BoundingBox.hpp"
#include "engine/primitives/Frustum.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

constexpr int height { 1080 };
constexpr int width { 1920 };
constexpr float ASPECT_RATIO { static_cast< float >( width ) / static_cast< float >( height ) };

TEST_CASE( "Frustum", "[frustum][rotation][translation]" )
{
	Camera camera {};
	camera.setPerspectiveProjection( 90.0f, ASPECT_RATIO, constants::NEAR_PLANE, constants::FAR_PLANE );

	GIVEN( "A default frustum from a default camera" )
	{
		const auto base_frustum { camera.getBaseFrustum() };

		THEN( "The near plane should have a distance of constants::NEAR_PLANE" )
		{
			REQUIRE( base_frustum.near.distance() == constants::NEAR_PLANE );
		}
		THEN( "The far plane should have a distance of -constants::FAR_PLANE" )
		{
			REQUIRE( base_frustum.far.distance() == -constants::FAR_PLANE );
		}
	}
}