//
// Created by kj16609 on 2/16/24.
//

#include <catch2/catch_all.hpp>

#include "engine/primitives/TransformComponent.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

bool epsilonEqual( const float a, const float b )
{
	return std::abs( a - b ) <= std::numeric_limits< float >::epsilon();
}

TEST_CASE( "Transform", "[transform][rotation][translation]" )
{
	TransformComponent component;

	component.translation = constants::WORLD_CENTER;
	component.scale = glm::vec3( 1.0f );
	component.rotation = Rotation( 0.0f );

	REQUIRE( component.mat4() == glm::mat4( 1.0f ) );

	constexpr auto TEST_POINT { constants::WORLD_FORWARD };

	SECTION( "Rotation" )
	{
		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_UP when pitched 90 degrees
		WHEN( "Rotated +90 Pitch" )
		{
			//Rotate by pitch
			component.rotation.pitch = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_UP" )
			{
				REQUIRE( rotated_point == constants::WORLD_UP );
			}
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_DOWN when pitched -90 degrees
		WHEN( "Rotated -90 Pitch" )
		{
			component.rotation.pitch = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_DOWN" )
			{
				REQUIRE( rotated_point == constants::WORLD_DOWN );
			}
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_RIGHT when yawed 90 degrees
		WHEN( "Rotated +90 Yaw" )
		{
			component.rotation.yaw = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_RIGHT" )
			{
				REQUIRE( rotated_point == constants::WORLD_RIGHT );
			}
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_LEFT when yawed -90 degrees
		WHEN( "Rotated -90 Yaw" )
		{
			component.rotation.yaw = -glm::radians( 90.0f );

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
			component.rotation.roll = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

			THEN( "Forward should be WORLD_DOWN" )
			{
				REQUIRE( rotated_point == constants::WORLD_DOWN );
			}
		}

		//Tests behaviour that a point from WORLD_RIGHT should end up WORLD_UP when rolled -90 degrees
		WHEN( "Rotated -90 Roll" )
		{
			component.rotation.roll = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

			THEN( "Forward should be WORLD_UP" )
			{
				REQUIRE( rotated_point == constants::WORLD_UP );
			}
		}
	}

	SECTION( "Translation" )
	{
		WHEN( "Translated X+" )
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

		WHEN( "Translated Z+" )
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
		WHEN( "Translated X+90 and Rotated Y+90" )
		{
			component.rotation.yaw = glm::radians( 90.0f );
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( constants::WORLD_FORWARD, 1.0f ) };

			THEN( "WORLD_FORWARD should be transformed into (WORLD_RIGHT * 2)" )
			{
				REQUIRE( translated_point == ( constants::WORLD_RIGHT * 2.0f ) );
			}
		}

		SECTION( "Yaw- X+" )
		{
			component.rotation.yaw = glm::radians( -90.0f );
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( translated_point.x );
			CAPTURE( translated_point.y );
			CAPTURE( translated_point.z );
			REQUIRE( epsilonEqual( translated_point.x, 0.0f ) );
			REQUIRE( epsilonEqual( translated_point.y, 0.0f ) );
			REQUIRE( epsilonEqual( translated_point.z, 0.0f ) );
		}

		SECTION( "Yaw+ Y+" )
		{
			component.rotation.yaw = glm::radians( 90.0f );
			component.translation.forward() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( translated_point.x );
			CAPTURE( translated_point.y );
			CAPTURE( translated_point.z );
			REQUIRE( epsilonEqual( translated_point.x, ( constants::WORLD_RIGHT * 1.0f ).x ) );
			REQUIRE( epsilonEqual( translated_point.y, ( constants::WORLD_FORWARD * 1.0f ).y ) );
			REQUIRE( epsilonEqual( translated_point.z, std::numeric_limits< float >::epsilon() ) );
		}
	}
}
