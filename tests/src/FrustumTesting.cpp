//
// Created by kj16609 on 2/15/24.
//

#include <catch2/catch_all.hpp>

#include <iostream>

#define EXPOSE_CAMERA_INTERNAL
#include "engine/Camera.hpp"
#include "engine/primitives/Frustum.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

constexpr float ASPECT_RATIO { 16.0f / 9.0f };

TEST_CASE( "Frustum", "[frustum][rotation][translation]" )
{
	Camera camera;
	camera.setPerspectiveProjection( 90.0f, ASPECT_RATIO, constants::NEAR_PLANE, constants::FAR_PLANE );

	const auto base_frustum { camera.getBaseFrustum() };

	REQUIRE( base_frustum.near.direction() == constants::WORLD_FORWARD );
	REQUIRE( base_frustum.near.distance() == constants::NEAR_PLANE );

	REQUIRE( base_frustum.far.direction() == constants::WORLD_BACKWARD );
	REQUIRE( base_frustum.far.distance() == -constants::FAR_PLANE );

	SECTION( "Translate" )
	{
		WHEN( "Translated backwards" )
		{
			camera.setViewYXZ( constants::WORLD_CENTER - constants::WORLD_FORWARD, Rotation( 0.0f, 0.0f, 0.0f ) );
			//Translate backwards by 1 world unit
			const auto translated_backwards { camera.getFrustumBounds() };

			//Verify that during a translation the direction isn't changed
			THEN( "Direction is the same" )
			{
				REQUIRE( translated_backwards.near.direction() == base_frustum.near.direction() );
			}

			THEN( "The near plane should be translated backwards" )
			{
				const glm::vec3 target_near { ( constants::WORLD_FORWARD * constants::NEAR_PLANE )
					                          - constants::WORLD_FORWARD };

				REQUIRE( translated_backwards.near.getPosition() == glm::vec3 { target_near } );
				REQUIRE( translated_backwards.near.direction() == constants::WORLD_FORWARD );
				REQUIRE( translated_backwards.near.distance() == constants::NEAR_PLANE - 1.0f );
			}

			THEN( "The far plane should be translated backwards" )
			{
				const glm::vec3 target_far { ( constants::WORLD_FORWARD * constants::FAR_PLANE )
					                         - constants::WORLD_FORWARD };

				REQUIRE( translated_backwards.far.getPosition() == target_far );
				REQUIRE( translated_backwards.far.direction() == constants::WORLD_BACKWARD );
				REQUIRE( translated_backwards.far.distance() == -( constants::FAR_PLANE - 1.0f ) );
				// The distance for the far plane should be negative. Due to the fact
				// that it is pointing toward the origin, So in order for the center to be positive
				// the distance must also be negative
			}
		}

		WHEN( "Translated Forward" )
		{
			camera.setViewYXZ( constants::WORLD_CENTER + constants::WORLD_FORWARD, Rotation( 0.0f, 0.0f, 0.0f ) );
			//Translate forward by 1 world unit
			const auto translated_forward { camera.getFrustumBounds() };

			THEN( "Direction is the same" )
			{
				//Verify that during a translation the direction isn't changed
				REQUIRE( translated_forward.near.direction() == base_frustum.near.direction() );
			}

			THEN( "The near plane should be translated backwards" )
			{
				REQUIRE( translated_forward.near.direction() == constants::WORLD_FORWARD );
				REQUIRE( translated_forward.near.distance() == constants::NEAR_PLANE + 1.0f );
				REQUIRE(
					translated_forward.near.getPosition()
					== ( constants::WORLD_FORWARD * ( constants::NEAR_PLANE + 1.0f ) ) );
			}

			THEN( "The far plane should be translated backwards" )
			{
				REQUIRE( translated_forward.far.direction() == constants::WORLD_BACKWARD );
				REQUIRE( translated_forward.far.distance() == -( constants::FAR_PLANE + 1.0f ) );
				REQUIRE(
					translated_forward.far.getPosition()
					== ( constants::WORLD_FORWARD * ( constants::FAR_PLANE + 1.0f ) ) );
			}
		}

		WHEN( "Translated Up" )
		{
			camera.setViewYXZ( constants::WORLD_CENTER + constants::WORLD_UP, Rotation( 0.0f, 0.0f, 0.0f ) );
			//Translate up by 1 world unit
			const auto translated_up { camera.getFrustumBounds() };

			THEN( "Direction is the same" )
			{
				//Verify that during a translation the direction isn't changed
				REQUIRE( translated_up.near.direction() == base_frustum.near.direction() );
			}

			REQUIRE( translated_up.near.direction() == constants::WORLD_FORWARD );
			REQUIRE( translated_up.near.distance() == constants::NEAR_PLANE );

			REQUIRE( translated_up.far.direction() == constants::WORLD_BACKWARD );
			REQUIRE( translated_up.far.distance() == -constants::FAR_PLANE );
		}
	}

	const Frustum< fgl::engine::CoordinateSpace::World > frustum { Matrix< MatrixType::ModelToWorld > { 1.0f }
		                                                           * camera.getBaseFrustum() };

	SECTION( "Far up test" )
	{
		//A point extremely far up should be in front of the bottom plane but behind the top plane
		const WorldCoordinate far_up { constants::WORLD_UP * 1000.0f };

		REQUIRE( frustum.top.distanceFrom( far_up ) < -500.0f );
		REQUIRE( frustum.bottom.distanceFrom( far_up ) > 500.0f );

		REQUIRE_FALSE( frustum.pointInside( far_up ) );
	}

	SECTION( "Far down test" )
	{
		//A point extremely far below should be in front of the top plane but behind the bottom plane
		const WorldCoordinate far_down { constants::WORLD_DOWN * 1000.0f };

		REQUIRE( frustum.top.distanceFrom( far_down ) > 500.0f );
		REQUIRE( frustum.bottom.distanceFrom( far_down ) < -500.0f );

		REQUIRE_FALSE( frustum.pointInside( far_down ) );
	}

	//The point FORWARD should be in the frustum
	SECTION( "Forward point test" )
	{
		const WorldCoordinate point { constants::WORLD_FORWARD * 2.0f };

		REQUIRE( frustum.near.distanceFrom( point ) > 0.0f );
		REQUIRE( frustum.far.distanceFrom( point ) > 0.0f );

		REQUIRE( frustum.top.distanceFrom( point ) > 0.0f );
		REQUIRE( frustum.bottom.distanceFrom( point ) > 0.0f );

		REQUIRE( frustum.right.distanceFrom( point ) > 0.0f );
		REQUIRE( frustum.left.distanceFrom( point ) > 0.0f );

		REQUIRE( frustum.pointInside( point ) );
	}

	//A point FORWARD and 0.2 units to the left should be farther from the right plane
	{
		const WorldCoordinate point { ( constants::WORLD_FORWARD * 2.0f ) + constants::WORLD_LEFT * 0.2f };

		REQUIRE( frustum.near.distanceFrom( point ) > 0.0f );
		REQUIRE( frustum.far.distanceFrom( point ) > 0.0f );

		REQUIRE( frustum.top.distanceFrom( point ) > 0.0f );
		REQUIRE( frustum.bottom.distanceFrom( point ) > 0.0f );

		REQUIRE( frustum.right.distanceFrom( point ) > 0.0f );
		REQUIRE( frustum.left.distanceFrom( point ) > 0.0f );

		// right_dist > left_dist
		REQUIRE( frustum.right.distanceFrom( point ) > frustum.left.distanceFrom( point ) );

		REQUIRE( frustum.pointInside( point ) );
	}

	//A point FORWARD and 0.2 units to the right should be farther from the left plane
	{
		const WorldCoordinate point { ( constants::WORLD_FORWARD * 2.0f ) + constants::WORLD_RIGHT * 0.2f };

		// left_dist > right_dist
		REQUIRE( frustum.left.distanceFrom( point ) > frustum.right.distanceFrom( point ) );
	}

	//A point FORWARD and down 0.2 units should be closer to the bottom plane
	{
		const WorldCoordinate point { ( constants::WORLD_FORWARD * 2.0f ) + ( constants::WORLD_DOWN * 0.02f ) };

		REQUIRE( frustum.top.distanceFrom( point ) > frustum.bottom.distanceFrom( point ) );
	}

	SECTION( "Rotations" )
	{
		//Testing rotation of the camera
		SECTION( "Pitch" )
		{
			Rotation rotation { 0.0f, 0.0f, 0.0f };
			rotation.pitch -= glm::radians( 90.0f );

			camera.setViewYXZ( constants::CENTER, rotation );

			const auto rotated_frustum = camera.getFrustumBounds();

			//The point DOWN should be in the frustum
			const WorldCoordinate point { constants::WORLD_DOWN * 2.0f };

			//NEAR should be looking down or approaching
			//FAR should be looking up or away

			//TODO: Rewrite these equals to allow for some mild precission errors
			//REQUIRE( rotated_frustum.near.direction(), constants::WORLD_DOWN );
			//REQUIRE( rotated_frustum.far.direction(), constants::WORLD_UP );

			REQUIRE( rotated_frustum.near.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.far.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.top.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.bottom.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.right.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.left.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.pointInside( point ) );
		}

		SECTION( "Yaw" )
		{
			Rotation rotation { 0.0f, 0.0f, 0.0f };
			rotation.yaw += glm::radians( 90.0f );

			camera.setViewYXZ( constants::CENTER, rotation );

			const auto rotated_frustum = camera.getFrustumBounds();

			//The point RIGHT should be in the frustum
			const WorldCoordinate point { constants::WORLD_RIGHT * 2.0f };

			//NEAR should be looking right or approaching
			//FAR should be looking left or away

			//Precision issues. So I can't write the tests yet

			REQUIRE( rotated_frustum.near.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.far.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.top.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.bottom.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.right.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.left.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.pointInside( point ) );

			//LEFT should be behind, Thus out outside
			REQUIRE_FALSE( rotated_frustum.pointInside( WorldCoordinate( constants::WORLD_LEFT ) ) );
		}

		SECTION( "Roll" )
		{
			Rotation rotation { 0.0f, 0.0f, 0.0f };
			rotation.roll -= glm::radians( 90.0f );

			camera.setViewYXZ( constants::CENTER, rotation );

			const auto rotated_frustum = camera.getFrustumBounds();

			//The point FORWARD should be in the frustum
			const WorldCoordinate point { constants::WORLD_FORWARD * 2.0f };

			REQUIRE( rotated_frustum.near.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.far.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.top.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.bottom.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.right.distanceFrom( point ) > 0.0f );
			REQUIRE( rotated_frustum.left.distanceFrom( point ) > 0.0f );

			REQUIRE( rotated_frustum.pointInside( point ) );
		}
	}
}