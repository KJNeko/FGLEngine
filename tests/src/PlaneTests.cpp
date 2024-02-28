//
// Created by kj16609 on 2/25/24.
//

#include <catch2/catch_all.hpp>

#include <iostream>

#define EXPOSE_CAMERA_INTERNAL
#include "engine/Camera.hpp"
#include "engine/primitives/Frustum.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

TEST_CASE( "Planes", "[frustum][rotation][translation]" )
{
	WHEN( "Given a default constructed Plane" )
	{
		Plane< CoordinateSpace::World > plane {};

		THEN( "Distance should be DEFAULT_FLOAT" )
		{
			REQUIRE( plane.distance() == constants::DEFAULT_FLOAT );
		}

		THEN( "Direction should be WORLD_FORWARD" )
		{
			REQUIRE( plane.direction() == constants::WORLD_FORWARD );
		}

		THEN( "Position should be Y+ infinity" )
		{
			REQUIRE( plane.getPosition() == constants::DEFAULT_VEC3 * constants::WORLD_FORWARD );
		}
	}

	GIVEN( "A Plane pointing at WORLD_FORWARD with a distance of 0.5" )
	{
		const Plane< CoordinateSpace::Model > plane { constants::WORLD_FORWARD, 0.5f };

		TransformComponent component { WorldCoordinate( constants::WORLD_CENTER ),
			                           glm::vec3( 1.0f ),
			                           { 0.0f, 0.0f, 0.0f } };

		WHEN( "Translated forward 1U" )
		{
			component.translation += constants::WORLD_FORWARD;
			const auto translated_plane { component.mat() * plane };

			THEN( "The distance should be 0.5 + 1" )
			{
				REQUIRE( translated_plane.distance() == 0.5f + 1.0f );
			}

			THEN( "The direction should be WORLD_BACKWARD" )
			{
				REQUIRE( translated_plane.direction() == constants::WORLD_FORWARD );
			}

			THEN( "The position should be 0.5U behind the origin" )
			{
				REQUIRE( translated_plane.getPosition() == constants::WORLD_FORWARD * 1.5f );
			}
		}

		WHEN( "Translated backwards 1U" )
		{
			component.translation -= constants::WORLD_FORWARD;
			const auto translated_plane { component.mat() * plane };

			THEN( "The distance should be 0.5 - 1" )
			{
				REQUIRE( translated_plane.distance() == 0.5f - 1.0f );
			}

			THEN( "The direction should be WORLD_BACKWARD" )
			{
				REQUIRE( translated_plane.direction() == constants::WORLD_FORWARD );
			}

			THEN( "The position should be 0.5U behind the origin" )
			{
				REQUIRE( translated_plane.getPosition() == constants::WORLD_FORWARD * -0.5f );
			}
		}

		WHEN( "Rotated +90 Yaw" )
		{
			component.translation = constants::WORLD_CENTER;
			component.rotation.yaw() += glm::radians( 90.0f );

			THEN( "Distance should not change" )
			{
				REQUIRE( ( component.mat() * plane ).distance() == plane.distance() );
			}

			THEN( "Direction should be WORLD_RIGHT" )
			{
				REQUIRE( ( component.mat() * plane ).direction() == constants::WORLD_RIGHT );
			}
		}

		WHEN( "Rotated +90 Yaw and translated 1U Right" )
		{
			component.translation += constants::WORLD_RIGHT;
			component.rotation.yaw() += glm::radians( 90.0f );

			const auto matrix { component.mat() };

			const auto translated_plane { matrix * plane };

			THEN( "new_direction should be WORLD_RIGHT" )
			{
				REQUIRE( matrix * plane.direction() == constants::WORLD_RIGHT );
			}

			REQUIRE( static_cast< glm::vec3 >( matrix * plane.getPosition() ) == constants::WORLD_RIGHT * 1.5f );

			THEN( "The distance should be 0.5 + 1" )
			{
				REQUIRE( translated_plane.distance() == 0.5f + 1.0f );
			}

			THEN( "The direction should be WORLD_RIGHT" )
			{
				REQUIRE( translated_plane.direction() == constants::WORLD_RIGHT );
			}

			THEN( "The position should be 0.5U behind the origin" )
			{
				REQUIRE( translated_plane.getPosition() == constants::WORLD_RIGHT * 1.5f );
			}
		}
	}
}

TEST_CASE( "Plane intersections", "[frustum][intersection]" )
{
	GIVEN( "A line going from -1 to 1 (X,Y and Z)" )
	{
		const LineSegment< CoordinateSpace::World > line { glm::vec3( -1 ), glm::vec3( 1 ) };

		AND_GIVEN( "A plane facing toward line.start" )
		{
			const Plane< CoordinateSpace::World > plane { glm::normalize( line.start ), 0.0f };

			THEN( "The line should intersect" )
			{
				REQUIRE( plane.intersects( line ) );

				AND_THEN( "The line intersection should be at origin" )
				{
					REQUIRE( plane.intersection( line ) == glm::vec3( 0.0f ) );
				}
			}

			THEN( "The line should intersect going the opposite way" )
			{
				REQUIRE( plane.intersects( line.flip() ) );

				AND_THEN( "The line intersection should be at origin" )
				{
					REQUIRE( plane.intersection( line ) == glm::vec3( 0.0f ) );
				}
			}
		}
	}

	GIVEN( "A line going from WORLD_LEFT - WORLD_FRONT * 20.0 to WORLD_LEFT + WORLD_FRONT * 20.0f" )
	{
		const LineSegment< CoordinateSpace::World > line { constants::WORLD_LEFT - ( constants::WORLD_FORWARD * 20.0f ),
			                                               constants::WORLD_LEFT + ( constants::WORLD_FORWARD * 20.0f ) };

		AND_GIVEN( "A plane facing toward WORLD_FORWARD at distance -2.0f" )
		{
			const Plane< CoordinateSpace::World > plane { constants::WORLD_FORWARD, -2.0f };

			THEN( "The plane should intersect the line" )
			{
				REQUIRE( plane.intersects( line ) );
			}

			THEN( "The intersection point should be WORLD_LEFT - (constants::WORLD_FORWARD * 2.0f)" )
			{
				REQUIRE( plane.intersection( line ) == constants::WORLD_LEFT - ( constants::WORLD_FORWARD * 2.0f ) );
			}
		}
	}

	GIVEN( "A line going from WORLD_UP to WORLD_DOWN" )
	{
		const LineSegment< fgl::engine::CoordinateSpace::World > line {
			constants::WORLD_UP * 5.0f - constants::WORLD_BACKWARD,
			constants::WORLD_DOWN * 5.0f
			                                                         - constants::WORLD_BACKWARD };

		AND_GIVEN( "A plane facing UP + FORWARD" )
		{
			const Plane< fgl::engine::CoordinateSpace::World > plane { constants::WORLD_FORWARD + constants::WORLD_UP,
				                                                       0.0f };

			THEN( "The plane should intersect the line" )
			{
				REQUIRE( plane.intersects( line ) );
			}
		}
	}
}
