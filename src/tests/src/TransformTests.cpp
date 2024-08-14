//
// Created by kj16609 on 2/16/24.
//

#include <catch2/catch_all.hpp>

#include "engine/primitives/TransformComponent.hpp"
#include "gtest_printers.hpp"
#include "operators/vector.hpp"

using namespace fgl::engine;

/*
TEST_CASE( "Matrix", "[transform][rotation][translation][matrix]" )
{
	TransformComponent component {};
	component.scale = glm::vec3( 8.0f );
	component.translation = WorldCoordinate( constants::WORLD_FORWARD * 100.0f );

	WHEN( "Rotation is set to 90.0f yaw" )
	{
		component.rotation = Rotation( 0.0f ).yaw() += glm::radians( 90.0f );

		GIVEN( "The matrix from the transform" )
		{
			const auto matrix { component.mat() };

			THEN( "The extracted rotation should match the transform rotation" )
			{
				REQUIRE( component.rotation.w == Catch::Approx( matrix.quat().w ) );
				REQUIRE( component.rotation.x == Catch::Approx( matrix.quat().x ) );
				REQUIRE( component.rotation.y == Catch::Approx( matrix.quat().y ) );
				REQUIRE( component.rotation.z == Catch::Approx( matrix.quat().z ) );
			}
		}
	}
}
*/

TEST_CASE( "Transform", "[transform][rotation][translation]" )
{
	TransformComponent component;

	component.translation = WorldCoordinate( constants::WORLD_CENTER );
	component.scale = glm::vec3( 1.0f );
	component.rotation = Rotation( 0.0f );

	constexpr auto TEST_POINT { constants::WORLD_FORWARD };

	GIVEN( "A default rotation" )
	{
		THEN( "The rotation is (0,0,0)" )
		{
			REQUIRE( component.rotation.pitch() == 0.0f );
			REQUIRE( component.rotation.yaw() == 0.0f );
			REQUIRE( component.rotation.roll() == 0.0f );
		}

		THEN( "The rotation matrix is the identity matrix" )
		{
			REQUIRE( constants::MAT4_IDENTITY == component.mat4() );
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_UP when pitched 90 degrees
		WHEN( "Rotated +90 Pitch" )
		{
			//Rotate by pitch
			component.rotation.pitch() = glm::radians( 90.0f );

			THEN( "Forward should be WORLD_UP" )
			{
				REQUIRE( component.forward() == NormalVector( constants::WORLD_UP ) );
			}

			THEN( "WORLD_FORWARD should be rotated to WORLD_UP" )
			{
				REQUIRE(
					component.rotation.mat() * NormalVector( constants::WORLD_FORWARD )
					== NormalVector( constants::WORLD_UP ) );
			}

			THEN( "Pitch should be 90.0f" )
			{
				REQUIRE( component.rotation.pitch() == glm::radians( 90.0f ) );
			}
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_DOWN when pitched -90 degrees
		WHEN( "Rotated -90 Pitch" )
		{
			component.rotation.pitch() = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_DOWN" )
			{
				REQUIRE( rotated_point == constants::WORLD_DOWN );
			}
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_RIGHT when yawed 90 degrees
		WHEN( "Rotated +90 Yaw" )
		{
			component.rotation.yaw() = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_RIGHT" )
			{
				REQUIRE( rotated_point == constants::WORLD_RIGHT );
			}
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_LEFT when yawed -90 degrees
		WHEN( "Rotated -90 Yaw" )
		{
			component.rotation.yaw() = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_LEFT" )
			{
				REQUIRE( rotated_point == constants::WORLD_LEFT );
			}
		}

		//Tests behaviour that a point from WORLD_RIGHT should end up WORLD_DOWN when rolled 90 degrees
		//This behaviour assumes that WORLD_RIGHT is 90 deg YAW+ from WORLD_FORWARD
		WHEN( "Rotated +90 Roll" )
		{
			component.rotation.roll() = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

			THEN( "Forward should be WORLD_DOWN" )
			{
				REQUIRE( rotated_point == constants::WORLD_DOWN );
			}
		}

		//Tests behaviour that a point from WORLD_RIGHT should end up WORLD_UP when rolled -90 degrees
		WHEN( "Rotated -90 Roll" )
		{
			component.rotation.roll() = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

			THEN( "Forward should be WORLD_UP" )
			{
				REQUIRE( rotated_point == constants::WORLD_UP );
			}
		}
	}

	SECTION( "Translation" )
	{
		WHEN( "Translated Z+" )
		{
			component.translation.up() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_FORWARD + WORLD_UP" )
			{
				REQUIRE( translated_point == constants::WORLD_FORWARD + constants::WORLD_UP );
			}
		}

		WHEN( "Translated Y+" )
		{
			component.translation.forward() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_FORWARD + WORLD_FORWARD" )
			{
				REQUIRE( translated_point == constants::WORLD_FORWARD + constants::WORLD_FORWARD );
			}
		}

		WHEN( "Translated X+" )
		{
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_FORWARD + WORLD_RIGHT" )
			{
				REQUIRE( translated_point == constants::WORLD_FORWARD + constants::WORLD_RIGHT );
			}
		}
	}

	SECTION( "Mix" )
	{
		WHEN( "Translated X+1 and Rotated Y+90" )
		{
			component.rotation.yaw() = glm::radians( glm::radians( 90.0f ) );
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( constants::WORLD_FORWARD, 1.0f ) };

			THEN( "WORLD_FORWARD should be transformed into (WORLD_RIGHT * 2)" )
			{
				REQUIRE( translated_point == ( constants::WORLD_RIGHT * 2.0f ) );
			}
		}

		SECTION( "Translated X+1 Yaw-90" )
		{
			component.rotation.yaw() = glm::radians( -glm::radians( 90.0f ) );
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "WORLD_FORWARD should be transformed into WORLD_CENTER)" )
			{
				REQUIRE( translated_point == constants::WORLD_CENTER );
			}
		}
	}
}
