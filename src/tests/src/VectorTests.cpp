//
// Created by kj16609 on 2/16/24.
//

#include <glm/gtc/epsilon.hpp>

#include <utility>

#include "engine/primitives/TransformComponent.hpp"
#include "engine/primitives/vectors/Vector.hpp"
#include "gtest_printers.hpp"
#include "operators/vector.hpp"

using namespace fgl::engine;

TEST_CASE( "Vector", "[vector][transforms]" )
{
	WHEN( "Vector is default constructed" )
	{
		constexpr Vector vec {};
		THEN( "X should be max floatf" )
		{
			REQUIRE( vec.x == constants::DEFAULT_FLOAT );
		}
		THEN( "Y should be max float" )
		{
			REQUIRE( vec.y == constants::DEFAULT_FLOAT );
		}
		THEN( "Z should be max float" )
		{
			REQUIRE( vec.z == constants::DEFAULT_FLOAT );
		}
	}

	WHEN( "Rotated +90 yaw" )
	{
		Vector rotation_vec { NormalVector( constants::WORLD_FORWARD ) };

		//Rotate by 90 degrees on yaw
		TransformComponent transform;
		transform.rotation.yaw() += glm::radians( 90.0f );

		const Vector value { transform.mat() * rotation_vec };

		THEN( "Forward should be WORLD_RIGHT" )
		{
			REQUIRE( value == Vector( constants::WORLD_RIGHT ) );
		}
	}

	WHEN( "Rotated -90 yaw" )
	{
		Vector rotation_vec { NormalVector( constants::WORLD_FORWARD ) };

		//Rotate by 90 degrees on yaw
		TransformComponent transform;
		transform.rotation.yaw() += glm::radians( -90.0f );

		const Vector value { transform.mat() * rotation_vec };

		THEN( "Forward should be WORLD_LEFT" )
		{
			REQUIRE( value == Vector( constants::WORLD_LEFT ) );
		}
	}
}

TEST_CASE( "Rotation", "[vector][transforms]" )
{
	WHEN( "Rotation is default constructed" )
	{
		Rotation rot {};

		THEN( "Should match a default quaternion" )
		{
			REQUIRE( static_cast< glm::quat >( rot ) == glm::quat( 1.0f, 0.0f, 0.0f, 0.0f ) );
		}

		THEN( "Yaw should be 0.0f" )
		{
			REQUIRE( rot.yaw() == 0.0f );
		}
		THEN( "Pitch should be 0.0f" )
		{
			REQUIRE( rot.pitch() == 0.0f );
		}
		THEN( "Roll should be 0.0f" )
		{
			REQUIRE( rot.roll() == 0.0f );
		}

		THEN( "Forward should be WORLD_FORWARD" )
		{
			REQUIRE( rot.forward() == NormalVector( constants::WORLD_FORWARD ) );
		}

		THEN( "Backwards should be WORLD_BACKWARD" )
		{
			REQUIRE( -rot.forward() == NormalVector( constants::WORLD_BACKWARD ) );
		}

		THEN( "Right should be WORLD_RIGHT" )
		{
			REQUIRE( rot.right() == NormalVector( constants::WORLD_RIGHT ) );
		}

		THEN( "Left should be WORLD_LEFT" )
		{
			REQUIRE( -rot.right() == NormalVector( constants::WORLD_LEFT ) );
		}
	}

	GIVEN( "A rotation constructed" )
	{
		constexpr auto rad_90 { glm::radians( 90.0f ) };

		AND_WHEN( "Given 90.0f pitch" )
		{
			Rotation rotation { rad_90, 0.0f, 0.0f };

			THEN( "Pitch should return 90" )
			{
				REQUIRE( rotation.pitch() == Catch::Approx( rad_90 ).epsilon( 0.01 ) );
			}

			/*
			THEN( "Quaternion should be valid" )
			{
				REQUIRE( static_cast< glm::quat >( rotation ) == glm::quat( 0.7071068, 0.7071068, 0, 0 ) );
			}
			*/
		}

		AND_WHEN( "Given 90.0f yaw" )
		{
			Rotation rotation { 0.0f, rad_90, 0.0f };

			REQUIRE( rotation.yaw() == Catch::Approx( rad_90 ).epsilon( 0.01 ) );
		}

		AND_WHEN( "Given 90.0f roll" )
		{
			Rotation rotation { 0.0f, 0.0f, rad_90 };

			REQUIRE( rotation.roll() == Catch::Approx( rad_90 ).epsilon( 0.01 ) );
		}
	}

	GIVEN( "A default identity matrix" )
	{
		const Matrix< MatrixType::ModelToWorld > matrix { 1.0f };

		THEN( "The matrix rot() function should match a default rotation" )
		{
			const Rotation default_rot {};
			REQUIRE( default_rot.mat() == matrix.rotmat() );
		}

		WHEN( "Multiplied with a default rotation" )
		{
			THEN( "Nothing should happen" )
			{}
		}
	}

	SECTION( "Rotation transforms" )
	{
		WHEN( "Rotated +90 yaw from default" )
		{
			Rotation rot { 0.0f, 0.0f, glm::radians( 90.0f ) };

			THEN( "Forward should be WORLD_RIGHT" )
			{
				REQUIRE( rot.forward() == NormalVector( constants::WORLD_RIGHT ) );
			}

			THEN( "Backwards should be WORLD_LEFT" )
			{
				REQUIRE( rot.back() == NormalVector( constants::WORLD_LEFT ) );
			}

			THEN( "Right should be WORLD_BACKWARD" )
			{
				REQUIRE( rot.right() == NormalVector( constants::WORLD_BACKWARD ) );
			}

			THEN( "Left should be WORLD_FORWARD" )
			{
				REQUIRE( rot.left() == NormalVector( constants::WORLD_FORWARD ) );
			}
		}

		WHEN( "Rotated -90 yaw from default" )
		{
			Rotation rot { 0.0f, 0.0f, glm::radians( -90.0f ) };

			THEN( "Forward should be WORLD_LEFT" )
			{
				REQUIRE( rot.forward() == NormalVector( constants::WORLD_LEFT ) );
			}

			THEN( "Backwards should be WORLD_RIGHT" )
			{
				REQUIRE( rot.back() == NormalVector( constants::WORLD_RIGHT ) );
			}

			THEN( "Right should be WORLD_FORWARD" )
			{
				REQUIRE( rot.right() == NormalVector( constants::WORLD_FORWARD ) );
			}

			THEN( "Left should be WORLD_BACKWARD" )
			{
				REQUIRE( rot.left() == NormalVector( constants::WORLD_BACKWARD ) );
			}
		}
	}
}
