//
// Created by kj16609 on 2/15/25.
//

#include "primitives/boxes/OrientedBoundingBox.hpp"

#include <catch2/catch_all.hpp>

#include "tests/src/gtest_printers.hpp"

#include <engine/math/intersections/obb/intersections.hpp>

TEST_CASE( "Oriented Bounding Box", "[intersection][obb][coordinate]" )
{
	using namespace fgl::engine;

	SECTION( "Coordinate" )
	{
		constexpr Coordinate< CoordinateSpace::World > start_pos { 1.0, 1.0, 1.0 };
		OrientedBoundingBox< CoordinateSpace::World > box { start_pos, glm::vec3( 1.0f ) };

		box.getTransform().rotation.addX( 2.0 );
		box.getTransform().rotation.addY( 2.0 );
		box.getTransform().rotation.addZ( 2.0 );

		WHEN( "Coordinate is outside the bounds of the box" )
		{
			Coordinate< CoordinateSpace::World > coord { 5.0, 5.0, 5.0 };

			THEN( "The intersection should fail" )
			{
				REQUIRE( !intersections::contains( box, coord ) );
			}
		}

		WHEN("Coordinate is inside of the bounds of the box")
		{
			Coordinate< CoordinateSpace::World > coord { 1.0, 1.0, 1.0 };
			THEN( "The intersection should pass" )
			{
				REQUIRE( intersections::contains( box, coord ) );
			}
		}
	}
}
