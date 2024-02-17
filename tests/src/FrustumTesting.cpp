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

TEST_CASE( "Frustum creation", "[frustum]" )
{
	Camera camera;
	camera.setPerspectiveProjection( 90.0f, ASPECT_RATIO, constants::NEAR_PLANE, constants::FAR_PLANE );

	const auto base_frustum = camera.getBaseFrustum();

	REQUIRE( camera.getRight() == constants::WORLD_RIGHT );

	REQUIRE( base_frustum.near.direction() == constants::WORLD_FORWARD );
	REQUIRE( base_frustum.near.distance() == constants::NEAR_PLANE );

	REQUIRE( base_frustum.far.direction() == constants::WORLD_BACKWARD );
	REQUIRE( base_frustum.far.distance() == -constants::FAR_PLANE );
}

TEST_CASE( "Frustum translations", "[frustum][translation]" )
{
	Camera camera;
	camera.setPerspectiveProjection( 90.0f, ASPECT_RATIO, constants::NEAR_PLANE, constants::FAR_PLANE );

	const auto base_frustum = camera.getBaseFrustum();
	glm::mat4 mat { 1.0f };

	REQUIRE( camera.getRight() == constants::WORLD_RIGHT );

	SECTION( "Translate" )
	{
		SECTION( "Backwards" )
		{
			//Translate backwards by 1 world unit
			mat = glm::translate( mat, glm::vec3( constants::WORLD_BACKWARD ) );
			const auto translated_backwards { Matrix< MatrixType::ModelToWorld >( mat ) * base_frustum };

			//Verify that during a translation the direction isn't changed
			REQUIRE( translated_backwards.near.direction() == base_frustum.near.direction() );

			REQUIRE( translated_backwards.near.direction() == constants::WORLD_FORWARD );
			REQUIRE( translated_backwards.near.distance() == constants::NEAR_PLANE - 1.0f );

			REQUIRE( translated_backwards.far.direction() == constants::WORLD_BACKWARD );
			REQUIRE( translated_backwards.far.distance() == -( constants::FAR_PLANE - 1.0f ) );
			// The distance for the far plane should be negative. Due to the fact
			// that it is poining toward the origin, So in order for the center to be positive
			// the distance must also be negative
		}

		SECTION( "Forward" )
		{
			//Translate forward by 1 world unit
			mat = glm::translate( glm::mat4( 1.0f ), glm::vec3( constants::WORLD_FORWARD ) );
			const auto translated_forward { Matrix< MatrixType::ModelToWorld >( mat ) * base_frustum };

			//Verify that during a translation the direction isn't changed
			REQUIRE( translated_forward.near.direction() == base_frustum.near.direction() );

			REQUIRE( translated_forward.near.direction() == constants::WORLD_FORWARD );
			REQUIRE( translated_forward.near.distance() == constants::NEAR_PLANE + 1.0f );

			REQUIRE( translated_forward.far.direction() == constants::WORLD_BACKWARD );
			REQUIRE( translated_forward.far.distance() == -( constants::FAR_PLANE + 1.0f ) );
		}

		SECTION( "Up" )
		{
			mat = glm::rotate( glm::mat4( 1.0f ), -glm::radians( 90.0f ), constants::WORLD_UP );

			const auto rotated_right { Matrix< MatrixType::ModelToWorld >( mat ) * base_frustum };

			REQUIRE( rotated_right.near.direction().x == constants::WORLD_RIGHT.x );
			REQUIRE( rotated_right.near.direction().y < 0.000001f ); // Precision issues. However it's VERY close to 0

			REQUIRE( rotated_right.near.distance() == constants::NEAR_PLANE );

			REQUIRE( rotated_right.far.direction().x == constants::WORLD_LEFT.x );
			REQUIRE( rotated_right.far.direction().y < 0.000001f ); // Precision issues. However it's VERY close to 0

			REQUIRE( rotated_right.far.distance() == -constants::FAR_PLANE );
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

	//Camera should be non-rotated at 0,0,0 so it should be identical
	REQUIRE( Matrix< MatrixType::ModelToWorld > { 1.0f } * camera.getBaseFrustum() == camera.getFrustumBounds() );

	SECTION( "Rotations" )
	{
		//Testing rotation of the camera
		SECTION( "Pitch" )
		{
			Vector rotation { 0.0f, 0.0f, 0.0f };
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
			Vector rotation { 0.0f, 0.0f, 0.0f };
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
			Vector rotation { 0.0f, 0.0f, 0.0f };
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