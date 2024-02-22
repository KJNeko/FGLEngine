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

		CAPTURE( camera.view_matrix );

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

		WHEN( "Camera is translated left by WORLD_LEFT" )
		{
			camera.setView( constants::WORLD_CENTER + constants::WORLD_LEFT, Rotation( 0.0f ) );

			THEN( "camera.getPosition() should be WORLD_UP" )
			{
				REQUIRE( camera.getPosition() == constants::WORLD_LEFT );
			}

			THEN( "A point at the origin should be translated to the right" )
			{
				const auto matrix { camera.getProjectionViewMatrix() };
				const glm::vec3 point { matrix * glm::vec4( constants::WORLD_CENTER, 1.0f ) };

				CAPTURE( matrix );
				CAPTURE( point );

				REQUIRE( point.x > 0.0f );
				REQUIRE( point.y == 0.0f );
			}
		}

		constexpr int window_width { 1920 };
		constexpr int window_height { 1080 };

		GIVEN( "A window height of 1920x1080" )
		{
			WHEN( "Camera is translated down by WORLD_DOWN " )
			{
				camera.setView(
					constants::WORLD_CENTER + constants::WORLD_DOWN + constants::WORLD_BACKWARD, Rotation( 0.0f ) );

				THEN( "camera.getPosition() should be WORLD_DOWN" )
				{
					REQUIRE( camera.getPosition() == constants::WORLD_DOWN + constants::WORLD_BACKWARD );
				}

				const auto view_matrix { camera.getViewMatrix() };
				const auto projection_matrix { camera.getProjectionMatrix() };
				const auto matrix { camera.getProjectionViewMatrix() };

				CAPTURE( view_matrix );
				CAPTURE( projection_matrix );
				CAPTURE( matrix );

				const glm::vec3 point { glm::projectZO(
					constants::WORLD_CENTER,
					glm::mat4( 1.0f ),
					matrix,
					glm::vec4( 0.0f, 0.0f, window_width, window_height ) ) };

				THEN( "A point should be above the half way point of the screen" )
				{
					CAPTURE( point );
					//Because vulkan starts at the top left, the y axis is inverted [0, 1] where 0 is the top. 1 is the bottom.
					// This means that a point 'below' the half way mark on the screen will actually be more positive.
					REQUIRE( point.y < window_height / 2.0f );
				}

				THEN( "A point should be in the view of the screen" )
				{
					REQUIRE( point.x >= 0.0f );
					REQUIRE( point.x <= window_width );
					REQUIRE( point.y >= 0.0f );
					REQUIRE( point.y <= window_height );
				}

				THEN( "A point should be infront of the camera" )
				{
					REQUIRE( point.z >= 0.0f );
					AND_THEN( "The point should not be in front of the camera" )
					{
						REQUIRE( point.z <= 1.0f );
					}
				}
			}

			WHEN( "Camera is translated up by WORLD_UP" )
			{
				camera.setView(
					constants::WORLD_CENTER + constants::WORLD_UP + constants::WORLD_BACKWARD, Rotation( 0.0f ) );

				THEN( "camera.getPosition() should be WORLD_UP" )
				{
					const auto position { camera.getPosition() };
					REQUIRE( position == constants::WORLD_UP + constants::WORLD_BACKWARD );
				}

				const auto view_matrix { camera.getViewMatrix() };
				const auto projection_matrix { camera.getProjectionMatrix() };
				const auto matrix { camera.getProjectionViewMatrix() };

				CAPTURE( view_matrix );
				CAPTURE( projection_matrix );
				CAPTURE( matrix );

				const glm::vec3 point { glm::projectZO(
					constants::WORLD_CENTER,
					glm::mat4( 1.0f ),
					matrix,
					glm::vec4( 0.0f, 0.0f, window_width, window_height ) ) };

				CAPTURE( point );

				THEN( "A point should be below the half way point of the screen" )
				{
					//Because vulkan starts at the top left, the y axis is inverted [0, 1] where 0 is the top. 1 is the bottom.
					// This means that a point 'below' the half way mark on the screen will actually be more positive.
					REQUIRE( point.y > ( window_height / 2 ) );
				}

				THEN( "A point should be in the view of the screen" )
				{
					REQUIRE( point.x >= 0.0f );
					REQUIRE( point.x <= window_width );
					REQUIRE( point.y >= 0.0f );
					REQUIRE( point.y <= window_height );
				}

				THEN( "A point should be in front of the camera" )
				{
					REQUIRE( point.z > 0.0f );
					AND_THEN( "The point should not be behind the camera" )
					{
						REQUIRE( point.z < 1.0f );
					}
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

				AND_WHEN( "A point is processed through the matrix" )
				{
					const auto matrix { camera.getProjectionViewMatrix() };
					const glm::vec3 point { glm::projectZO(
						constants::WORLD_CENTER,
						glm::mat4( 1.0f ),
						matrix,
						glm::vec4( 0.0f, 0.0f, window_width, window_height ) ) };

					THEN( "A point at the origin should be translated back" )
					{
						CAPTURE( point );

						THEN( "Point should be in the view of the screen" )
						{
							REQUIRE( point.x >= 0.0f );
							REQUIRE( point.x <= window_width );
							REQUIRE( point.y >= 0.0f );
							REQUIRE( point.y <= window_height );
						}

						THEN( "Point should be behind the camera" )
						{
							//Comparing to 1 since the point should be behind the camera (0 to 1 is 'forward' to Z plane. Anything Above 1 is behind)
							REQUIRE( point.z > 1.0f );
						}
					}
				}
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