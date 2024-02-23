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

TEST_CASE( "Planes", "[frustum][rotation][translation]" )
{
	WHEN( "Given a default constructed Plane" )
	{
		Plane< CoordinateSpace::World > plane {};

		THEN( "Distance should be DEFAULT_FLOAT" )
		{
			REQUIRE( plane.distance() == constants::DEFAULT_FLOAT );
		}

		THEN( "Direction should be WORLD_FORWARD" )
		{
			REQUIRE( plane.direction() == constants::WORLD_FORWARD );
		}

		THEN( "Position should be Y+ infinity" )
		{
			REQUIRE( plane.getPosition() == constants::DEFAULT_VEC3 * constants::WORLD_FORWARD );
		}
	}

	GIVEN( "A Plane pointing at WORLD_FORWARD with a distance of 0.5" )
	{
		const Plane< CoordinateSpace::Model > plane { constants::WORLD_FORWARD, 0.5f };

		TransformComponent component { WorldCoordinate( constants::WORLD_CENTER ),
			                           glm::vec3( 1.0f ),
			                           { 0.0f, 0.0f, 0.0f } };

		WHEN( "Translated forward 1U" )
		{
			component.translation += constants::WORLD_FORWARD;
			const auto translated_plane { component.mat() * plane };

			THEN( "The distance should be 0.5 + 1" )
			{
				REQUIRE( translated_plane.distance() == 0.5f + 1.0f );
			}

			THEN( "The direction should be WORLD_BACKWARD" )
			{
				REQUIRE( translated_plane.direction() == constants::WORLD_FORWARD );
			}

			THEN( "The position should be 0.5U behind the origin" )
			{
				REQUIRE( translated_plane.getPosition() == constants::WORLD_FORWARD * 1.5f );
			}
		}

		WHEN( "Translated backwards 1U" )
		{
			component.translation -= constants::WORLD_FORWARD;
			const auto translated_plane { component.mat() * plane };

			THEN( "The distance should be 0.5 - 1" )
			{
				REQUIRE( translated_plane.distance() == 0.5f - 1.0f );
			}

			THEN( "The direction should be WORLD_BACKWARD" )
			{
				REQUIRE( translated_plane.direction() == constants::WORLD_FORWARD );
			}

			THEN( "The position should be 0.5U behind the origin" )
			{
				REQUIRE( translated_plane.getPosition() == constants::WORLD_FORWARD * -0.5f );
			}
		}

		WHEN( "Rotated +90 Yaw" )
		{
			component.translation = constants::WORLD_CENTER;
			component.rotation.yaw() += glm::radians( 90.0f );

			THEN( "Distance should not change" )
			{
				REQUIRE( ( component.mat() * plane ).distance() == plane.distance() );
			}

			THEN( "Direction should be WORLD_RIGHT" )
			{
				REQUIRE( ( component.mat() * plane ).direction() == constants::WORLD_RIGHT );
			}
		}

		WHEN( "Rotated +90 Yaw and translated 1U Right" )
		{
			component.translation += constants::WORLD_RIGHT;
			component.rotation.yaw() += glm::radians( 90.0f );

			const auto matrix { component.mat() };

			const auto translated_plane { matrix * plane };

			THEN( "new_direction should be WORLD_RIGHT" )
			{
				REQUIRE( matrix * plane.direction() == constants::WORLD_RIGHT );
			}

			REQUIRE( static_cast< glm::vec3 >( matrix * plane.getPosition() ) == constants::WORLD_RIGHT * 1.5f );

			THEN( "The distance should be 0.5 + 1" )
			{
				REQUIRE( translated_plane.distance() == 0.5f + 1.0f );
			}

			THEN( "The direction should be WORLD_RIGHT" )
			{
				REQUIRE( translated_plane.direction() == constants::WORLD_RIGHT );
			}

			THEN( "The position should be 0.5U behind the origin" )
			{
				REQUIRE( translated_plane.getPosition() == constants::WORLD_RIGHT * 1.5f );
			}
		}
	}
}

constexpr int height { 1080 };
constexpr int width { 1920 };
constexpr float ASPECT_RATIO { static_cast< float >( width ) / static_cast< float >( height ) };

TEST_CASE( "Frustum", "[frustum][rotation][translation]" )
{
	Camera camera;
	camera.setPerspectiveProjection( 90.0f, ASPECT_RATIO, constants::NEAR_PLANE, constants::FAR_PLANE );

	const auto base_frustum { camera.getBaseFrustum() };

	GIVEN( "A default camera with a 90 deg fov and a 1920/1080 aspect ratio." )
	{
		const auto frustum { camera.getFrustumBounds() };
		THEN( "The near plane should be facing WORLD_FORWARD" )
		{
			REQUIRE( frustum.near.direction() == constants::WORLD_FORWARD );
			REQUIRE( frustum.near.distance() == constants::NEAR_PLANE );
		}

		THEN( "The far plane should be facing WORLD_BACKWARD" )
		{
			REQUIRE( frustum.far.direction() == constants::WORLD_BACKWARD );
			REQUIRE( frustum.far.distance() == -constants::FAR_PLANE );
		}
	}

	WHEN( "Camera is translated backwards" )
	{
		camera.setView( constants::WORLD_BACKWARD, Rotation( 0.0f, 0.0f, 0.0f ) );
		//Translate backwards by 1 world unit
		const auto translated_backwards { camera.getFrustumBounds() };

		//Verify that during a translation the direction isn't changed
		THEN( "Direction is the same" )
		{
			REQUIRE( translated_backwards.near.direction() == base_frustum.near.direction() );
		}

		const auto matrix { camera.frustumTranslationMatrix() };
		const auto plane { camera.getBaseFrustum().near };

		const Vector new_direction { glm::normalize( matrix * plane.direction() ) };
		const glm::vec3 new_center { matrix * plane.getPosition() };
		const float new_distance { glm::dot( new_center, static_cast< glm::vec3 >( new_direction ) ) };

		CAPTURE( plane.direction() );
		CAPTURE( new_direction );
		CAPTURE( plane.getPosition() );
		CAPTURE( new_center );
		CAPTURE( new_distance );

		REQUIRE( new_distance != constants::NEAR_PLANE );

		THEN( "The near plane distance should be 1U less" )
		{
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
		camera.setView( constants::WORLD_CENTER + constants::WORLD_FORWARD, Rotation( 0.0f, 0.0f, 0.0f ) );
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
		camera.setView( constants::WORLD_CENTER + constants::WORLD_UP, Rotation( 0.0f, 0.0f, 0.0f ) );
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

	const Frustum< fgl::engine::CoordinateSpace::World > frustum { Matrix< MatrixType::ModelToWorld > { 1.0f }
		                                                           * camera.getBaseFrustum() };

	WHEN( "A point is above the frustum" )
	{
		const WorldCoordinate far_up { constants::WORLD_UP * 1000.0f };

		THEN( "The point should be outside the frustum" )
		{
			REQUIRE_FALSE( frustum.pointInside( far_up ) );
		}

		THEN( "The point should be infront of the bottom plane" )
		{
			REQUIRE( frustum.bottom.distanceFrom( far_up ) > 500.0f );
		}

		THEN( "The point should be behind the top plane" )
		{
			REQUIRE( frustum.top.distanceFrom( far_up ) < -500.0f );
		}
	}

	WHEN( "A point is below the frustum" )
	{
		const WorldCoordinate far_down { constants::WORLD_DOWN * 1000.0f };

		THEN( "The point should be outside the frustum" )
		{
			REQUIRE_FALSE( frustum.pointInside( far_down ) );
		}

		THEN( "The point should be infront of the top plane" )
		{
			REQUIRE( frustum.top.distanceFrom( far_down ) > 500.0f );
		}

		THEN( "The point should be behind the bottom plane" )
		{
			REQUIRE( frustum.bottom.distanceFrom( far_down ) < -500.0f );
		}
	}

	//The point FORWARD should be in the frustum
	WHEN( "A point infront of the frustum" )
	{
		GIVEN( "A distance of 2 units" )
		{
			const WorldCoordinate point { constants::WORLD_FORWARD * 2.0f };

			THEN( "The point should be inside the frustum" )
			{
				REQUIRE( frustum.pointInside( point ) );
			}
		}

		GIVEN( "A distance of 500 units" )
		{
			const WorldCoordinate point { constants::WORLD_FORWARD * 500.0f };

			THEN( "The point should be behind the far plane" )
			{
				CAPTURE( frustum.far.distanceFrom( point ) );
				REQUIRE( frustum.far.distanceFrom( point ) < 0.0f );
				REQUIRE_FALSE( frustum.far.isForward( point ) );
			}

			THEN( "The point should be outside the frustum" )
			{
				REQUIRE_FALSE( frustum.pointInside( point ) );
			}
		}
	}

	//A point FORWARD and 0.2 units to the left should be farther from the right plane
	GIVEN( "A point FORWARD and 0.2 units to the left" )
	{
		const WorldCoordinate point { ( constants::WORLD_FORWARD * 2.0f ) + constants::WORLD_LEFT * 0.2f };

		THEN( "The point should be farther from the right plane" )
		{
			REQUIRE( frustum.right.distanceFrom( point ) > frustum.left.distanceFrom( point ) );
		}

		THEN( "The point should be in the frustum" )
		{
			REQUIRE( frustum.pointInside( point ) );
		}
	}

	//A point FORWARD and 0.2 units to the right should be farther from the left plane
	GIVEN( "A point FORWARD and 0.2 units to the right" )
	{
		const WorldCoordinate point { ( constants::WORLD_FORWARD * 2.0f ) + constants::WORLD_RIGHT * 0.2f };

		THEN( "The point should be farther from the left plane" )
		{
			REQUIRE( frustum.left.distanceFrom( point ) > frustum.right.distanceFrom( point ) );
		}

		THEN( "The point should be in the frustum" )
		{
			REQUIRE( frustum.pointInside( point ) );
		}
	}

	//A point FORWARD and down 0.2 units should be closer to the bottom plane
	GIVEN( "A point FORWARD and down 0.2 units" )
	{
		const WorldCoordinate point { ( constants::WORLD_FORWARD * 2.0f ) + ( constants::WORLD_DOWN * 0.2f ) };

		THEN( "The point should be closer to the bottom plane" )
		{
			REQUIRE( frustum.bottom.distanceFrom( point ) < frustum.top.distanceFrom( point ) );
		}

		THEN( "The point should be in the frustum" )
		{
			REQUIRE( frustum.pointInside( point ) );
		}
	}

	SECTION( "Rotations" )
	{
		//Testing rotation of the camera
		SECTION( "Pitch" )
		{
			Rotation rotation { 0.0f, 0.0f, 0.0f };
			rotation.pitch() -= glm::radians( 90.0f );

			camera.setView( constants::CENTER, rotation );

			const auto rotated_frustum = camera.getFrustumBounds();

			//The point DOWN should be in the frustum
			const WorldCoordinate point { constants::WORLD_DOWN * 2.0f };

			//NEAR should be looking down or approaching
			//FAR should be looking up or away

			REQUIRE( rotated_frustum.near.direction() == constants::WORLD_DOWN );
			REQUIRE( rotated_frustum.far.direction() == constants::WORLD_UP );

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
			rotation.yaw() += glm::radians( 90.0f );

			camera.setView( constants::CENTER, rotation );

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
			rotation.roll() -= glm::radians( 90.0f );

			camera.setView( constants::CENTER, rotation );

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