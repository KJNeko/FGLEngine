//
// Created by kj16609 on 2/16/24.
//

#include <catch2/catch_all.hpp>

#include "engine/primitives/TransformComponent.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

TEST_CASE( "Transform rotations", "[transform][rotation]" )
{
	TransformComponent component;

	component.translation = constants::WORLD_CENTER;
	component.scale = glm::vec3( 1.0f );
	component.rotation = Vector( 0.0f );

	REQUIRE( component.mat4() == glm::mat4( 1.0f ) );

	//This should pitch any point by 90 degrees (Roughly equal to WORLD_UP)
	constexpr auto TEST_POINT { constants::WORLD_FORWARD };

	SECTION( "Pitch+ (UP)" )
	{
		//Rotate by pitch
		component.rotation.pitch = glm::radians( 90.0f );

		const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

		//Must be dot here since the precision isn't good enough to be exact.
		// If the dot product is close to 1, then the vectors are close to being equal
		REQUIRE( glm::dot( rotated_point, constants::WORLD_UP ) > 0.99f );
	}

	SECTION( "Pitch- (DOWN)" )
	{
		component.rotation.pitch = -glm::radians( 90.0f );

		const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

		REQUIRE( glm::dot( rotated_point, constants::WORLD_DOWN ) > 0.99f );
	}

	SECTION( "Yaw+ (RIGHT)" )
	{
		component.rotation.yaw = glm::radians( 90.0f );

		const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

		REQUIRE( glm::dot( rotated_point, constants::WORLD_RIGHT ) > 0.99f );
	}

	SECTION( "Yaw- (LEFT)" )
	{
		component.rotation.yaw = -glm::radians( 90.0f );

		const glm::vec3 rotated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

		REQUIRE( glm::dot( rotated_point, constants::WORLD_LEFT ) > 0.99f );
	}

	//Roll must be tested some other way. The testing point should be to the right, And instead should become WORLD_UP or WORLD_DOWN (WORLD_UP for -roll, WORLD_DOWN for +roll)
	SECTION( "Roll+ (FORWARD)" )
	{
		component.rotation.roll = glm::radians( 90.0f );

		const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

		REQUIRE( glm::dot( rotated_point, constants::WORLD_UP ) > 0.99f );
	}

	SECTION( "Roll- (BACKWARD)" )
	{
		component.rotation.roll = -glm::radians( 90.0f );

		const glm::vec3 rotated_point { component.mat4() * glm::vec4( constants::WORLD_RIGHT, 1.0f ) };

		REQUIRE( glm::dot( rotated_point, constants::WORLD_DOWN ) > 0.99f );
	}
}
