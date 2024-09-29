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

	SECTION( "X Rotation (Roll)" )
	{
		for ( float i = 0; i < 360.0f; i += 1.0f )
		{
			GIVEN( "A rotation with euler angles (" << i << ", 0.0f, 0.0f)" )
			{
				Rotation rot { glm::radians( i ), 0.0f, 0.0f };

				INFO( "Euler Value: " << i );

				INFO( "Euler X: " << rot.xAngle() );
				INFO( "Euler Y: " << rot.yAngle() );
				INFO( "Euler Z: " << rot.zAngle() );

				INFO(
					"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
							  << ", " << rot.internal_quat().z << ")" );

				if ( i < 180.0f )
				{
					// Test must be disabled due to gimbol lock when converting to euler being weird above 180.0f
					THEN( "Euler X should be " << i )
					{
						REQUIRE( rot.xAngle() == Catch::Approx( glm::radians( i ) ) );
						REQUIRE( rot.yAngle() == Catch::Approx( 0.0 ) );
						REQUIRE( rot.zAngle() == Catch::Approx( 0.0 ) );
					}
				}

				THEN( "forward() should not change throughout the rotation" )
				{
					const NormalVector point { rot.forward() };

					REQUIRE( point == NormalVector( constants::WORLD_FORWARD ) );
				}

				THEN( "A point starting at WORLD_RIGHT should rotate to WORLD_UP when given X+ (Roll)" )
				{
					const NormalVector point { rot.right() };

					// When rotating around the X axis, the right vector should rotate to the up vector
					// This means that the x value should be at 0.0 through the entire rotation
					REQUIRE( point.x == Catch::Approx( 0.0 ).margin( 0.1 ) );

					if ( i == 0.0f )
					{
						// At 0.0f, the right vector should be the same as the WORLD_RIGHT vector
						REQUIRE( point == NormalVector( constants::WORLD_RIGHT ) );
					}
					else if ( i == 90.0f )
					{
						// At a 90+ rotation the point should be at WORLD_UP
						REQUIRE( point == NormalVector( constants::WORLD_UP ) );
					}
					else if ( i == 180.0f )
					{
						// A full 180 degree rotation should bring the point back to WORLD_LEFT
						REQUIRE( point == NormalVector( -constants::WORLD_RIGHT ) );
					}
					else if ( i == 270.0f )
					{
						// A full 270 degree rotation should bring the point back to WORLD_DOWN
						REQUIRE( point == NormalVector( -constants::WORLD_UP ) );
					}
					else if ( i == 360.0f )
					{
						// A full 360 degree rotation should bring the point back to WORLD_RIGHT
						REQUIRE( point == NormalVector( constants::WORLD_RIGHT ) );
					}
				}
			}
		}
	}

	SECTION( "Y Rotation (Pitch)" )
	{
		for ( float i = 0; i < 360.0f; i += 1.0f )
		{
			GIVEN( "A rotation with euler angles (0.0f, " << i << ", 0.0f)" )
			{
				//TODO: Figure out how to test for 180.0f deg
				Rotation rot { 0.0f, glm::radians( i ), 0.0f };

				INFO( "Euler Value: " << i );

				INFO( "Euler X: " << rot.xAngle() );
				INFO( "Euler Y: " << rot.yAngle() );
				INFO( "Euler Z: " << rot.zAngle() );

				INFO(
					"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
							  << ", " << rot.internal_quat().z << ")" );

				if ( i < 90.0f )
				{
					// Test must be disabled due to gimbol lock when converting to euler above 90.0f
					THEN( "Euler Y should be " << i )
					{
						REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ).epsilon( 0.1 ) );
						REQUIRE( rot.yAngle() == Catch::Approx( glm::radians( i ) ).epsilon( 0.01 ) );
						REQUIRE( rot.zAngle() == Catch::Approx( glm::radians( 0.0 ) ).epsilon( 0.01 ) );
					}
				}

				THEN( "right() should return WORLD_RIGHT throughout the rotation" )
				{
					REQUIRE( rot.right() == NormalVector( constants::WORLD_RIGHT ) );
				}

				THEN( "A point starting at WORLD_FORWARD should rotate to WORLD_UP when given Y+ (pitch)" )
				{
					const NormalVector point { rot.forward() };

					// When rotating around the Y axis, the forward vector should rotate to the left vector
					// This means that the x value should be at 0.0 through the entire rotation
					REQUIRE( point.y == Catch::Approx( 0.0 ).margin( 0.1 ) );

					INFO( "Point: " << point.x << ", " << point.y << ", " << point.z );

					if ( i == 0.0f )
					{
						// At 0.0f, the forward vector should be the same as the WORLD_FORWARD vector
						REQUIRE( point == NormalVector( constants::WORLD_FORWARD ) );
					}
					else if ( i == 90.0f )
					{
						// At a 90+ rotation the point should be at WORLD_LEFT
						REQUIRE( point == NormalVector( constants::WORLD_UP ) );
					}
					else if ( i == 180.0f )
					{
						// A full 180 degree rotation should bring the point back to WORLD_BACKWARD
						REQUIRE( point == NormalVector( -constants::WORLD_FORWARD ) );
					}
					else if ( i == 270.0f )
					{
						// A full 270 degree rotation should bring the point back to WORLD_RIGHT
						REQUIRE( point == NormalVector( -constants::WORLD_UP ) );
					}
					else if ( i == 360.0f )
					{
						// A full 360 degree rotation should bring the point back to WORLD_FORWARD
						REQUIRE( point == NormalVector( constants::WORLD_FORWARD ) );
					}
				}
			}
		}
	}

	SECTION( "Z Rotation (Yaw)" )
	{
		for ( float i = 0; i < 360.0f; i += 1.0f )
		{
			GIVEN( "A rotation with euler angles (0.0f, 0.0f, " << i << " )" )
			{
				Rotation rot { 0.0f, 0.0f, glm::radians( i ) };

				INFO( "Euler Value: " << i );

				INFO( "Euler X: " << rot.xAngle() );
				INFO( "Euler Y: " << rot.yAngle() );
				INFO( "Euler Z: " << rot.zAngle() );

				INFO(
					"Quat: (" << rot.internal_quat().w << ", " << rot.internal_quat().x << ", " << rot.internal_quat().y
							  << ", " << rot.internal_quat().z << ")" );

				if ( i < 90.0f )
				{
					// Test must be disabled due to gimbol lock when converting to euler
					THEN( "Euler Z should be " << i )
					{
						REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ) );
						REQUIRE( rot.yAngle() == Catch::Approx( 0.0 ) );
						REQUIRE( rot.zAngle() == Catch::Approx( glm::radians( i ) ) );
					}
				}

				THEN( "up() should not change throughout the rotation" )
				{
					REQUIRE( rot.up() == NormalVector( constants::WORLD_UP ) );
				}

				THEN( "A point starting at WORLD_FORWARD should rotate to WORLD_UP when given Z+ (pitch)" )
				{
					const NormalVector point { rot.forward() };

					// When rotating around the Z axis, the up vector should rotate to the left vector
					// This means that the x value should be at 0.0 through the entire rotation
					REQUIRE( point.z == Catch::Approx( 0.0 ).margin( 0.1 ) );

					INFO( "Point: " << point.x << ", " << point.y << ", " << point.z );

					if ( i == 0.0f )
					{
						// At 0.0f, the forward vector should be the same as the WORLD_FORWARD vector
						REQUIRE( point == NormalVector( constants::WORLD_FORWARD ) );
					}
					else if ( i == 90.0f )
					{
						// At a 90+ rotation the point should be at WORLD_UP
						REQUIRE( point == NormalVector( constants::WORLD_RIGHT ) );
					}
					else if ( i == 180.0f )
					{
						// A full 180 degree rotation should bring the point back to WORLD_BACKWARD
						REQUIRE( point == NormalVector( constants::WORLD_BACKWARD ) );
					}
					else if ( i == 270.0f )
					{
						// A full 270 degree rotation should bring the point back to WORLD_DOWN
						REQUIRE( point == NormalVector( constants::WORLD_LEFT ) );
					}
					else if ( i == 360.0f )
					{
						// A full 360 degree rotation should bring the point back to WORLD_FORWARD
						REQUIRE( point == NormalVector( constants::WORLD_FORWARD ) );
					}
				}
			}
		}
	}

	SECTION( "Y+Z Rotation" )
	{
		//TODO: Figure out how to test for 180.0f deg
		for ( float i = 0; i < 90.0f; i += 1.0f )
		{
			GIVEN( "A rotation with euler angles (0.0f, Y, Z)" )
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
					REQUIRE( rot.xAngle() == Catch::Approx( 0.0 ).epsilon( 0.1 ).margin( 0.1 ) );
					REQUIRE( rot.yAngle() == Catch::Approx( glm::radians( i ) ).epsilon( 0.01 ) );
					REQUIRE( rot.zAngle() == Catch::Approx( glm::radians( i ) ).epsilon( 0.01 ) );
				}
			}
		}
	}
}