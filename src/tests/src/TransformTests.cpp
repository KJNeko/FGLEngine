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

/*
TEST_CASE( "Quaternion Rotations", "[rotation]" )
{
	SECTION( "Quaternion build from euler" )
	{
		// euler should be in `pitch, roll, yaw` order
		const auto q_pitch { glm::angleAxis( 0.0f, constants::WORLD_RIGHT ) };
		const auto q_roll { glm::angleAxis( 0.0f, constants::WORLD_FORWARD ) };
		const auto q_yaw { glm::angleAxis( 0.0f, constants::WORLD_UP ) };

		REQUIRE( q_pitch == glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
		REQUIRE( q_pitch * q_roll == glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
		REQUIRE( q_pitch * q_roll * q_yaw == glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );

		const Rotation rot { 0.0f, 0.0f, 0.0f };

		REQUIRE( glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) == rot );
	}

	WHEN( "Building a quat with a 90.0f deg pitch" )
	{
		const glm::quat quat { fgl::engine::buildQuat( glm::radians( 90.0f ), 0.0f, 0.0f ) };

		THEN( "The output of glm::yaw should be 90.0f deg" )
		{
			REQUIRE( glm::pitch( quat ) == Catch::Approx( glm::radians( 90.0f ) ) );
			REQUIRE( glm::roll( quat ) == 0.0f );
			REQUIRE( glm::yaw( quat ) == 0.0f );
		}
	}

	WHEN( "Building a quat with a 90.0f deg roll" )
	{
		const glm::quat quat { fgl::engine::buildQuat( 0.0f, glm::radians( 90.0f ), 0.0f ) };

		THEN( "The output of glm::roll should be 90.0f deg" )
		{
			REQUIRE( glm::pitch( quat ) == 0.0f );
			REQUIRE( glm::roll( quat ) == Catch::Approx( glm::radians( 90.0f ) ) );
			REQUIRE( glm::yaw( quat ) == 0.0f );
		}
	}

	WHEN( "Building a quat with a 90.0f deg yaw" )
	{
		const glm::quat quat { fgl::engine::buildQuat( 0.0f, 0.0f, glm::radians( 90.0f ) ) };

		THEN( "The output of glm::pitch should be 90.0f deg" )
		{
			REQUIRE( glm::pitch( quat ) == 0.0f );
			REQUIRE( glm::roll( quat ) == 0.0f );
			REQUIRE( glm::yaw( quat ) == Catch::Approx( glm::radians( 90.0f ) ).epsilon( 0.01f ) );
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

	constexpr auto TEST_POINT { constants::WORLD_Y };

	SECTION( "Translation" )
	{
		WHEN( "Translated Z+" )
		{
			component.translation.up() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_FORWARD + WORLD_UP" )
			{
				REQUIRE( translated_point == constants::WORLD_Y + constants::WORLD_Z );
			}
		}

		WHEN( "Translated Y+" )
		{
			component.translation.forward() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_FORWARD + WORLD_FORWARD" )
			{
				REQUIRE( translated_point == constants::WORLD_Y + constants::WORLD_Y );
			}
		}

		WHEN( "Translated X+" )
		{
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "Forward should be WORLD_FORWARD + WORLD_RIGHT" )
			{
				REQUIRE( translated_point == constants::WORLD_Y + constants::WORLD_X );
			}
		}
	}

	/*
	SECTION( "Mix" )
	{
		WHEN( "Translated X+1 and Rotated Y+90" )
		{
			component.rotation.yAngle() = glm::radians( glm::radians( 90.0f ) );
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( constants::WORLD_Y, 1.0f ) };

			THEN( "WORLD_FORWARD should be transformed into (WORLD_RIGHT * 2)" )
			{
				REQUIRE( translated_point == ( constants::WORLD_X * 2.0f ) );
			}
		}

		SECTION( "Translated X+1 Yaw-90" )
		{
			component.rotation.yAngle() = glm::radians( -glm::radians( 90.0f ) );
			component.translation.right() += 1.0f;

			const glm::vec3 translated_point { component.mat4() * glm::vec4( TEST_POINT, 1.0f ) };

			THEN( "WORLD_FORWARD should be transformed into WORLD_CENTER)" )
			{
				REQUIRE( translated_point == constants::WORLD_CENTER );
			}
		}
	}
	*/
}