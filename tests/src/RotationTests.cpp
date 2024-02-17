//
// Created by kj16609 on 2/16/24.
//

#include <catch2/catch_all.hpp>

#include "engine/primitives/TransformComponent.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

std::ostream& operator<<( std::ostream& os, const glm::vec3 vec )
{
	os << "X: " << vec.x << " Y: " << vec.y << " Z: " << vec.z;
	return os;
}

TEST_CASE( "Transform", "[transform][rotation][translation]" )
{
	TransformComponent component;

	component.translation = constants::WORLD_CENTER;
	component.scale = glm::vec3( 1.0f );
	component.rotation = Vector( 0.0f );

	REQUIRE( component.mat4() == glm::mat4( 1.0f ) );

	constexpr auto TEST_POINT { constants::WORLD_FORWARD };

	SECTION( "Rotation" )
	{
		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_UP when pitched 90 degrees
		SECTION( "Pitch+ (UP)" )
		{
			//Rotate by pitch
			component.rotation.pitch = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			//Must be dot here since the precision isn't good enough to be exact.
			// If the dot product is close to 1, then the vectors are close to being equal
			CAPTURE( rotated_point.x );
			CAPTURE( rotated_point.y );
			CAPTURE( rotated_point.z );
			REQUIRE( glm::dot( rotated_point, constants::WORLD_UP ) > 0.99f );
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_DOWN when pitched -90 degrees
		SECTION( "Pitch- (DOWN)" )
		{
			component.rotation.pitch = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( rotated_point.x );
			CAPTURE( rotated_point.y );
			CAPTURE( rotated_point.z );
			REQUIRE( glm::dot( rotated_point, constants::WORLD_DOWN ) > 0.99f );
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_RIGHT when yawed 90 degrees
		SECTION( "Yaw+ (RIGHT)" )
		{
			component.rotation.yaw = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( rotated_point.x );
			CAPTURE( rotated_point.y );
			CAPTURE( rotated_point.z );
			REQUIRE( glm::dot( rotated_point, constants::WORLD_RIGHT ) > 0.99f );
		}

		// Tests behaviour that a point from WORLD_FORWARD should end up WORLD_LEFT when yawed -90 degrees
		SECTION( "Yaw- (LEFT)" )
		{
			component.rotation.yaw = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( rotated_point.x );
			CAPTURE( rotated_point.y );
			CAPTURE( rotated_point.z );
			REQUIRE( glm::dot( rotated_point, constants::WORLD_LEFT ) > 0.99f );
		}

		//Tests behaviour that a point from WORLD_RIGHT should end up WORLD_DOWN when rolled 90 degrees
		//This behaviour assumes that WORLD_RIGHT is 90 deg YAW+ from WORLD_FORWARD
		SECTION( "Roll+ (ROLL RIGHT)" )
		{
			component.rotation.roll = glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

			CAPTURE( rotated_point.x );
			CAPTURE( rotated_point.y );
			CAPTURE( rotated_point.z );
			REQUIRE( glm::dot( rotated_point, constants::WORLD_DOWN ) > 0.99f );
		}

		//Tests behaviour that a point from WORLD_RIGHT should end up WORLD_UP when rolled -90 degrees
		SECTION( "Roll- (ROLL LEFT)" )
		{
			component.rotation.roll = -glm::radians( 90.0f );

			const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

			CAPTURE( rotated_point.x );
			CAPTURE( rotated_point.y );
			CAPTURE( rotated_point.z );
			REQUIRE( glm::dot( rotated_point, constants::WORLD_UP ) > 0.99f );
		}
	}

	SECTION( "Translation" )
	{
		SECTION( "X+ Right" )
		{
			component.translation.up() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( translated_point.x );
			CAPTURE( translated_point.y );
			CAPTURE( translated_point.z );
			REQUIRE( translated_point == glm::vec3( 0.0f, 1.0f, 1.0f ) );
		}

		SECTION( "Y+ Forward" )
		{
			component.translation.forward() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( translated_point.x );
			CAPTURE( translated_point.y );
			CAPTURE( translated_point.z );
			REQUIRE( translated_point == glm::vec3( 0.0f, 2.0f, 0.0f ) );
		}

		SECTION( "Z+ Up" )
		{
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			CAPTURE( translated_point.x );
			CAPTURE( translated_point.y );
			CAPTURE( translated_point.z );
			REQUIRE( translated_point == glm::vec3( 1.0f, 1.0f, 0.0f ) );
		}
	}
}
