//
// Created by kj16609 on 1/30/25.
//

#include <catch2/catch_all.hpp>

#include "tests/src/gtest_printers.hpp"

using namespace Catch::literals;

TEST_CASE( "Quaternions", "[math][rotation]" )
{
	SECTION( "Default" )
	{
		fgl::engine::Rotation rot {};

		SECTION( "X Euler" )
		{
			REQUIRE( rot.xAngle() == 0.0f );
		}

		SECTION( "Y Euler" )
		{
			REQUIRE( rot.yAngle() == 0.0f );
		}

		SECTION( "Z Euler" )
		{
			REQUIRE( rot.zAngle() == 0.0f );
		}

		SECTION( "Raw" )
		{
			const glm::quat raw_quat { rot.internal_quat() };
			REQUIRE( raw_quat.x == 0.0f );
			REQUIRE( raw_quat.y == 0.0f );
			REQUIRE( raw_quat.z == 0.0f );
			REQUIRE( raw_quat.w == 1.0f );
		}
	}

	SECTION( "Predefined" )
	{
		SECTION( "-1.126, 1.012, -0.548" )
		{
			fgl::engine::Rotation rot { -1.256f, 1.012f, -0.548 };

			REQUIRE_THAT( rot.internal_quat().w, Catch::Matchers::WithinRel( 0.604f, 0.1f ) );
			REQUIRE_THAT( rot.internal_quat().x, Catch::Matchers::WithinRel( -0.601f, 0.1f ) );
			REQUIRE_THAT( rot.internal_quat().y, Catch::Matchers::WithinRel( -0.239f, 0.1f ) );
			REQUIRE_THAT( rot.internal_quat().z, Catch::Matchers::WithinRel( -0.466f, 0.1f ) );
		}
	}

	SECTION( "Singularity" )
	{
		SECTION( "North" )
		{
			fgl::engine::Rotation rot { 0.0f, 90.0f, 0.0f };
			REQUIRE_THAT( rot.euler().x, Catch::Matchers::WithinRel( 0.0f, 0.1f ) );
			REQUIRE_THAT( rot.euler().y, Catch::Matchers::WithinRel( 90.0f, 0.1f ) );
			REQUIRE_THAT( rot.euler().z, Catch::Matchers::WithinRel( 0.0f, 0.1f ) );
		}

		SECTION( "South" )
		{
			fgl::engine::Rotation rot { 0.0f, -90.0f, 0.0f };

			REQUIRE_THAT( rot.euler().x, Catch::Matchers::WithinRel( 0.0f, 0.1f ) );
			REQUIRE_THAT( rot.euler().y, Catch::Matchers::WithinRel( 90.0f, 0.1f ) );
			REQUIRE_THAT( rot.euler().z, Catch::Matchers::WithinRel( 0.0f, 0.1f ) );
		}
	}

	SECTION( "Beyond Singularity Y" )
	{
		fgl::engine::Rotation rot { 0.0f, 1.548f, 0.0f };

		REQUIRE_THAT( rot.euler().x, Catch::Matchers::WithinRel( -3.142f, 0.1f ) );
		REQUIRE_THAT( rot.euler().y, Catch::Matchers::WithinRel( 1.548f, 0.1f ) );
		REQUIRE_THAT( rot.euler().z, Catch::Matchers::WithinRel( -3.142f, 0.1f ) );
	}
}
