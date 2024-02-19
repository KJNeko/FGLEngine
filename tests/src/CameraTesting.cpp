//
// Created by kj16609 on 2/15/24.
//

#include <catch2/catch_all.hpp>

#include <iostream>

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

	camera.setPerspectiveProjection( 90.0f, 1.0f, constants::NEAR_PLANE, constants::FAR_PLANE );

	WHEN( "Camera is it's default orientation" )
	{
		camera.setView( constants::WORLD_CENTER, Rotation( 0.0f ) );

		THEN( "Camera up is WORLD_UP" )
		{
			const auto camera_up { camera.getUp() };
			REQUIRE( camera_up == constants::WORLD_UP );
		}

		THEN( "Camera right is WORLD_RIGHT" )
		{
			const auto camera_right { camera.getRight() };
			REQUIRE( camera_right == constants::WORLD_RIGHT );
		}

		THEN( "Camera forward is WORLD_FORWARD" )
		{
			const auto camera_forward { camera.getForward() };
			REQUIRE( camera_forward == constants::WORLD_FORWARD );
		}
	}

	SECTION( "Rotations" )
	{
		Rotation rotation_vec { 0.0f };
		WHEN( "Camera is rotated +90 Yaw" )
		{
			rotation_vec.yaw() = glm::radians( 90.0f );
			camera.setView( constants::WORLD_CENTER, rotation_vec );
			const auto camera_forward { camera.getForward() };

			REQUIRE( camera_forward == constants::WORLD_RIGHT );
		}

		WHEN( "Camera is rotated -90 Yaw" )
		{
			rotation_vec.yaw() = glm::radians( -90.0f );
			camera.setView( constants::WORLD_CENTER, rotation_vec );
			const auto camera_forward { camera.getForward() };

			REQUIRE( camera_forward == constants::WORLD_LEFT );
		}
	}

	SECTION( "Camera projection test - Perspective" )
	{
		WHEN( "Camera is rotated" )
		{
			float x_gen { GENERATE( 0.0f, 90.0f, 180.0f, 270.0f ) };
			float y_gen { GENERATE( 0.0f, 90.0f, 180.0f, 270.0f ) };
			float z_gen { GENERATE( 0.0f, 90.0f, 180.0f, 270.0f ) };

			const Rotation rotation { x_gen, y_gen, z_gen };

			camera.setView( constants::WORLD_RIGHT, rotation );

			THEN( "Camera translation should not change" )
			{
				const Coordinate< CoordinateSpace::World > position { camera.getPosition() };
				REQUIRE( position == constants::WORLD_RIGHT );
			}
		}

		WHEN( "Camera is translated right by WORLD_RIGHT" )
		{
			camera.setView( constants::WORLD_CENTER + constants::WORLD_RIGHT, Rotation( 0.0f ) );

			THEN( "camera.getPosition() should be WORLD_RIGHT" )
			{
				const auto position { camera.getPosition() };
				REQUIRE( position == constants::WORLD_RIGHT );
			}

			THEN( "A point at the origin should be translated to the left" )
			{
				const glm::mat4 combined_matrix { camera.getProjectionViewMatrix() };

				const glm::vec3 point { constants::WORLD_CENTER };

				const glm::vec3 translated_point { combined_matrix * glm::vec4( point, 1.0f ) };

				const auto projection_matrix { static_cast< glm::mat4 >( camera.getProjectionMatrix() ) };
				const auto view_matrix { static_cast< glm::mat4 >( camera.getViewMatrix() ) };

				CAPTURE( point );
				CAPTURE( projection_matrix );
				CAPTURE( view_matrix );
				CAPTURE( combined_matrix );
				CAPTURE( translated_point );

				REQUIRE( translated_point.x < 0.0f );
				REQUIRE( translated_point.y == 0.0f );
			}
		}

		WHEN( "Camera is translated up by WORLD_UP" )
		{
			camera.setView( constants::WORLD_CENTER + constants::WORLD_UP, Rotation( 0.0f ) );

			THEN( "camera.getPosition() should be WORLD_UP" )
			{
				const auto position { camera.getPosition() };
				REQUIRE( position == constants::WORLD_UP );
			}

			THEN( "A point at the origin should be translated down" )
			{
				const auto matrix { camera.getProjectionViewMatrix() };
				const glm::vec3 point { matrix * glm::vec4( constants::WORLD_CENTER, 1.0f ) };

				CAPTURE( point );

				REQUIRE( point.x == 0.0f );
				REQUIRE( point.y < 0.0f );
			}
		}

		WHEN( "Camera is translated forward by WORLD_FORWARD" )
		{
			camera.setView( constants::WORLD_CENTER + constants::WORLD_FORWARD, Rotation( 0.0f ) );

			THEN( "camera.getPosition() should be WORLD_FORWARD" )
			{
				const auto position { camera.getPosition() };
				REQUIRE( position == constants::WORLD_FORWARD );
			}

			THEN( "A point at the origin should be translated back" )
			{
				const auto matrix { camera.getProjectionViewMatrix() };
				const auto point { matrix * glm::vec4( constants::WORLD_CENTER, 1.0f ) };

				REQUIRE( point.x <= 0.0f );
				REQUIRE( point.y <= 0.0f );
			}
		}
	}

	WHEN( "Two points are translated by the camera when directly in front" )
	{
		const auto point_near { constants::WORLD_FORWARD };
		const auto point_far { constants::WORLD_FORWARD * 2.0f };

		const auto projected_near { camera.getProjectionViewMatrix() * glm::vec4( point_near, 1.0f ) };
		const auto projected_far { camera.getProjectionViewMatrix() * glm::vec4( point_far, 1.0f ) };

		THEN( "The near point should be closer than the far point" )
		{
			REQUIRE( projected_near.z < projected_far.z );
		}
	}
}