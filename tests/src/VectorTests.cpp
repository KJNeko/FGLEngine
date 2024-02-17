//
// Created by kj16609 on 2/16/24.
//

#include <glm/gtc/epsilon.hpp>

#include "engine/primitives/TransformComponent.hpp"
#include "engine/primitives/Vector.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

namespace Catch
{
	template <>
	struct StringMaker< Rotation >
	{
		static std::string convert( const Rotation& value )
		{
			return StringMaker< glm::vec3 >::convert( static_cast< glm::vec3 >( value ) );
		}
	};

} // namespace Catch

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
		Vector rotation_vec { constants::WORLD_FORWARD };

		//Rotate by 90 degrees on yaw
		TransformComponent transform;
		transform.rotation.yaw += glm::radians( 90.0f );

		const Vector value { transform.mat() * rotation_vec };

		THEN( "Forward should be WORLD_RIGHT" )
		{
			REQUIRE( value == constants::WORLD_RIGHT );
		}
	}

	WHEN( "Rotated -90 yaw" )
	{
		Vector rotation_vec { constants::WORLD_FORWARD };

		//Rotate by 90 degrees on yaw
		TransformComponent transform;
		transform.rotation.yaw += glm::radians( -90.0f );

		const Vector value { transform.mat() * rotation_vec };

		THEN( "Forward should be WORLD_LEFT" )
		{
			REQUIRE( value == constants::WORLD_LEFT );
		}
	}
}

TEST_CASE( "Rotation", "[vector][transforms]" )
{
	WHEN( "Rotation is default constructed" )
	{
		Rotation rot {};
		THEN( "Yaw should be 0.0f" )
		{
			REQUIRE( rot.yaw == 0.0f );
		}
		THEN( "Pitch should be 0.0f" )
		{
			REQUIRE( rot.pitch == 0.0f );
		}
		THEN( "Roll should be 0.0f" )
		{
			REQUIRE( rot.roll == 0.0f );
		}

		THEN( "Forward should be WORLD_FORWARD" )
		{
			REQUIRE( rot.forward() == constants::WORLD_FORWARD );
		}

		THEN( "Backwards should be WORLD_BACKWARD" )
		{
			REQUIRE( rot.backwards() == constants::WORLD_BACKWARD );
		}

		THEN( "Right should be WORLD_RIGHT" )
		{
			REQUIRE( rot.right( constants::WORLD_UP ) == constants::WORLD_RIGHT );
		}

		THEN( "Left should be WORLD_LEFT" )
		{
			REQUIRE( rot.left( constants::WORLD_UP ) == constants::WORLD_LEFT );
		}
	}

	SECTION( "Rotation transforms" )
	{
		WHEN( "Rotated +90 yaw from default" )
		{
			Rotation rot { 0.0f, 0.0f, glm::radians( 90.0f ) };

			THEN( "Forward should be WORLD_RIGHT" )
			{
				REQUIRE( rot.forward() == constants::WORLD_RIGHT );
			}

			THEN( "Backwards should be WORLD_LEFT" )
			{
				REQUIRE( rot.backwards() == constants::WORLD_LEFT );
			}

			THEN( "Right should be WORLD_BACKWARD" )
			{
				REQUIRE( rot.right( constants::WORLD_UP ) == constants::WORLD_BACKWARD );
			}

			THEN( "Left should be WORLD_FORWARD" )
			{
				REQUIRE( rot.left( constants::WORLD_UP ) == constants::WORLD_FORWARD );
			}
		}

		WHEN( "Rotated -90 yaw from default" )
		{
			Rotation rot { 0.0f, 0.0f, glm::radians( -90.0f ) };

			THEN( "Forward should be WORLD_LEFT" )
			{
				REQUIRE( rot.forward() == constants::WORLD_LEFT );
			}

			THEN( "Backwards should be WORLD_RIGHT" )
			{
				REQUIRE( rot.backwards() == constants::WORLD_RIGHT );
			}

			THEN( "Right should be WORLD_FORWARD" )
			{
				REQUIRE( rot.right( constants::WORLD_UP ) == constants::WORLD_FORWARD );
			}

			THEN( "Left should be WORLD_BACKWARD" )
			{
				REQUIRE( rot.left( constants::WORLD_UP ) == constants::WORLD_BACKWARD );
			}
		}
	}
}
