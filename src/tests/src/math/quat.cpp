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
		fgl::engine::QuatRotation rot {};

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
			fgl::engine::QuatRotation rot { -1.256f, 1.012f, -0.548 };

			REQUIRE_THAT( rot.internal_quat().w, Catch::Matchers::WithinAbs( 0.604f, 0.1f ) );
			REQUIRE_THAT( rot.internal_quat().x, Catch::Matchers::WithinAbs( -0.601f, 0.1f ) );
			REQUIRE_THAT( rot.internal_quat().y, Catch::Matchers::WithinAbs( -0.239f, 0.1f ) );
			REQUIRE_THAT( rot.internal_quat().z, Catch::Matchers::WithinAbs( -0.466f, 0.1f ) );
		}
	}

	SECTION( "Test xRange" )
	{
		for ( std::size_t i = 0; i < 360; ++i )
		{
			DYNAMIC_SECTION( "X Angle" << i )
			{
				constexpr float offset { std::numbers::pi_v< float > / 360.0f };
				fgl::engine::QuatRotation rot1 { static_cast< float >( i ) * offset, 0.0f, 0.0f };

				fgl::engine::QuatRotation rot2 { 0.0f, 0.0f, 0.0f };
				for ( std::size_t x = 0; x < i; ++x ) rot2.addX( offset );

				// compare both quaternions
				using namespace Catch::Matchers;

				REQUIRE_THAT( rot1.internal_quat().x, WithinAbs( rot2.internal_quat().x, 0.01f ) );
				REQUIRE_THAT( rot1.internal_quat().y, WithinAbs( rot2.internal_quat().y, 0.01f ) );
				REQUIRE_THAT( rot1.internal_quat().z, WithinAbs( rot2.internal_quat().z, 0.01f ) );
				REQUIRE_THAT( rot1.internal_quat().w, WithinAbs( rot2.internal_quat().w, 0.01f ) );
			}
		}

		SECTION( "Test yRange" )
		{
			for ( std::size_t i = 0; i < 360; ++i )
			{
				DYNAMIC_SECTION( "Y Angle: " << i )
				{
					constexpr float offset { std::numbers::pi_v< float > / 360.0f };
					fgl::engine::QuatRotation rot1 { 0.0f, static_cast< float >( i ) * offset, 0.0f };

					fgl::engine::QuatRotation rot2 { 0.0f, 0.0f, 0.0f };
					for ( std::size_t x = 0; x < i; ++x ) rot2.addY( offset );

					// compare both quaternions
					using namespace Catch::Matchers;

					REQUIRE_THAT( rot1.internal_quat().x, WithinAbs( rot2.internal_quat().x, 0.01f ) );
					REQUIRE_THAT( rot1.internal_quat().y, WithinAbs( rot2.internal_quat().y, 0.01f ) );
					REQUIRE_THAT( rot1.internal_quat().z, WithinAbs( rot2.internal_quat().z, 0.01f ) );
					REQUIRE_THAT( rot1.internal_quat().w, WithinAbs( rot2.internal_quat().w, 0.01f ) );
				}
			}
		}

		SECTION( "Test zRange" )
		{
			for ( std::size_t i = 0; i < 360; ++i )
			{
				DYNAMIC_SECTION( "Z Angle" << i )
				{
					constexpr float offset { std::numbers::pi_v< float > / 360.0f };
					fgl::engine::QuatRotation rot1 { 0.0f, 0.0f, static_cast< float >( i ) * offset };

					fgl::engine::QuatRotation rot2 { 0.0f, 0.0f, 0.0f };
					for ( std::size_t x = 0; x < i; ++x ) rot2.addZ( offset );

					// compare both quaternions
					using namespace Catch::Matchers;

					REQUIRE_THAT( rot1.internal_quat().x, WithinAbs( rot2.internal_quat().x, 0.01f ) );
					REQUIRE_THAT( rot1.internal_quat().y, WithinAbs( rot2.internal_quat().y, 0.01f ) );
					REQUIRE_THAT( rot1.internal_quat().z, WithinAbs( rot2.internal_quat().z, 0.01f ) );
					REQUIRE_THAT( rot1.internal_quat().w, WithinAbs( rot2.internal_quat().w, 0.01f ) );
				}
			}
		}
	}
}
