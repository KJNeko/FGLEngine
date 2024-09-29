//
// Created by kj16609 on 9/28/24.
//

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "engine/primitives/Rotation.hpp"
#include "gtest_printers.hpp"
#include "operators/vector.hpp"

TEST_CASE( "Rotations", "[transform][rotation]" )
{
	using namespace fgl::engine;

	GIVEN( "A Default Rotation" )
	{
		Rotation rot {};

		SECTION( "Check quat" )
		{
			glm::quat quat { rot.internal_quat() };

			THEN( "Quat: w should be 1.0" )
			{
				REQUIRE( quat.w == Catch::Approx( 1.0 ) );
			}

			auto is_zero { Catch::Approx( 0.0 ) };

			THEN( "Quat: x should be zero" )
			{
				REQUIRE( quat.x == is_zero );
			}
			THEN( "Quat: y should be zero" )
			{
				REQUIRE( quat.y == is_zero );
			}
			THEN( "Quat: z should be zero" )
			{
				REQUIRE( quat.z == is_zero );
			}
		}

		THEN( "Forward should be WORLD_FORWARD" )
		{
			REQUIRE( rot.forward() == NormalVector( constants::WORLD_FORWARD ) );
		}

		THEN( "Right should be WORLD_RIGHT" )
		{
			REQUIRE( rot.right() == NormalVector( constants::WORLD_RIGHT ) );
		}

		THEN( "Up should be WORLD_UP" )
		{
			REQUIRE( rot.up() == NormalVector( constants::WORLD_UP ) );
		}

		THEN( "Euler X should be 0.0" )
		{
			REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ) );
		}

		THEN( "Euler Y should be 0.0" )
		{
			REQUIRE( rot.yAngle() == Catch::Approx( 0.0 ) );
		}

		THEN( "Euler Z should be 0.0" )
		{
			REQUIRE( rot.zAngle() == Catch::Approx( 0.0 ) );
		}
	}

	GIVEN( "A rotation with euler angles (X, 0.0f, 0.0f)" )
	{
		for ( float i = 0; i < 180.0f; i += 1.0f )
		{
			Rotation rot { glm::radians( i ), 0.0f, 0.0f };

			INFO( "Euler Value: " << i );

			INFO( "Euler X: " << rot.xAngle() );
			INFO( "Euler Y: " << rot.yAngle() );
			INFO( "Euler Z: " << rot.zAngle() );

			INFO(
				"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
						  << ", " << rot.internal_quat().z << ")" );

			THEN( "Euler X should be " << i )
			{
				REQUIRE( rot.xAngle() == Catch::Approx( glm::radians( i ) ) );
				REQUIRE( rot.yAngle() == Catch::Approx( 0.0 ) );
				REQUIRE( rot.zAngle() == Catch::Approx( 0.0 ) );
			}
		}
	}

	GIVEN( "A rotation with euler angles (0.0f, Y, 0.0f)" )
	{
		for ( float i = 0; i < 180.0f; i += 1.0f )
		{
			Rotation rot { 0.0f, glm::radians( i ), 0.0f };

			INFO( "Euler Value: " << i );

			INFO( "Euler X: " << rot.xAngle() );
			INFO( "Euler Y: " << rot.yAngle() );
			INFO( "Euler Z: " << rot.zAngle() );

			INFO(
				"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
						  << ", " << rot.internal_quat().z << ")" );

			THEN( "Euler Y should be " << i )
			{
				REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ).epsilon( 0.1 ) );
				REQUIRE( rot.yAngle() == Catch::Approx( glm::radians( i ) ).epsilon( 0.01 ) );
				REQUIRE( rot.zAngle() == Catch::Approx( glm::radians( 0.0 ) ).epsilon( 0.01 ) );
			}
		}
	}

	GIVEN( "A rotation with euler angles (0.0f, 0.0f, Z)" )
	{
		for ( float i = 0; i < 180.0f; i += 1.0f )
		{
			Rotation rot { 0.0f, 0.0f, glm::radians( i ) };

			INFO( "Euler Value: " << i );

			INFO( "Euler X: " << rot.xAngle() );
			INFO( "Euler Y: " << rot.yAngle() );
			INFO( "Euler Z: " << rot.zAngle() );

			INFO(
				"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
						  << ", " << rot.internal_quat().z << ")" );

			THEN( "Euler Z should be " << i )
			{
				REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ) );
				REQUIRE( rot.yAngle() == Catch::Approx( 0.0 ) );
				REQUIRE( rot.zAngle() == Catch::Approx( glm::radians( i ) ) );
			}
		}
	}

	GIVEN( "A rotation with euler angles (0.0f, Y, Z)" )
	{
		for ( float i = 0; i < 180.0f; i += 1.0f )
		{
			Rotation rot { 0.0f, glm::radians( i ), glm::radians( i ) };

			INFO( "Euler Value: " << i );

			INFO( "Euler X: " << rot.xAngle() );
			INFO( "Euler Y: " << rot.yAngle() );
			INFO( "Euler Z: " << rot.zAngle() );

			INFO(
				"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
						  << ", " << rot.internal_quat().z << ")" );

			THEN( "Euler Y should be " << i )
			{
				REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ).epsilon( 0.1 ) );
				REQUIRE( rot.yAngle() == Catch::Approx( glm::radians( i ) ).epsilon( 0.01 ) );
				REQUIRE( rot.zAngle() == Catch::Approx( glm::radians( i ) ).epsilon( 0.01 ) );
			}
		}
	}
}