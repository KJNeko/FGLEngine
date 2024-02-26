//
// Created by kj16609 on 2/15/24.
//

#include <catch2/catch_all.hpp>

#include <iostream>

#define EXPOSE_FRUSTUM_INTERNALS
#define EXPOSE_CAMERA_INTERNAL
#include "engine/Camera.hpp"
#include "engine/model/BoundingBox.hpp"
#include "engine/primitives/Frustum.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

constexpr int height { 1080 };
constexpr int width { 1920 };
constexpr float ASPECT_RATIO { static_cast< float >( width ) / static_cast< float >( height ) };

TEST_CASE( "Frustum", "[frustum][rotation][translation]" )
{
	Camera camera {};
	camera.setPerspectiveProjection( 90.0f, ASPECT_RATIO, constants::NEAR_PLANE, constants::FAR_PLANE );

	GIVEN( "A default frustum from a default camera" )
	{
		const auto base_frustum { camera.getBaseFrustum() };
		const auto frustum { camera.getFrustumBounds() };

		THEN( "The near plane should have a distance of constants::NEAR_PLANE" )
		{
			REQUIRE( base_frustum.near.distance() == constants::NEAR_PLANE );
		}
		THEN( "The far plane should have a distance of -constants::FAR_PLANE" )
		{
			REQUIRE( base_frustum.far.distance() == -constants::FAR_PLANE );
		}

		GIVEN( "A line going from UP BACK to DOWN BACK" )
		{
			const Line< CoordinateSpace::World > line { constants::WORLD_BACKWARD + ( constants::WORLD_UP * 1.0f ),
				                                        constants::WORLD_BACKWARD + ( constants::WORLD_DOWN * 1.0f ) };

			THEN( "The signed distance of the end to the start should be positive" )
			{
				REQUIRE( signedDistance( line.direction(), line.end, line.start ) > 0.0f );
			}

			THEN( "The signed distance of the flipped line end multiplied by 2 should be negative" )
			{
				const WorldCoordinate new_end { line.flip().end * 2.0f };
				REQUIRE( signedDistance( line.direction(), new_end, line.start ) < 0.0f );
			}

			THEN( "The line should intersect the top and bottom planes" )
			{
				const auto top_intersection_point { frustum.top.intersection( line ) };

				AND_THEN( "The line should intersect the top plane BELOW the origin" )
				{
					REQUIRE_FALSE( std::isnan( top_intersection_point.z ) );
					REQUIRE( top_intersection_point.z < 0.0f );
				}

				const auto bottom_intersection_point { frustum.bottom.intersection( line ) };

				AND_THEN( "The line should intersect the bottom plane ABOVE the origin" )
				{
					REQUIRE_FALSE( std::isnan( top_intersection_point.z ) );
					REQUIRE( bottom_intersection_point.z > 0.0f );
				}

				const float top_signed_distance {
					signedDistance( line.direction(), top_intersection_point, line.start )
				};
				const float bottom_signed_distance {
					signedDistance( line.direction(), bottom_intersection_point, line.start )
				};

				//In this case the top plane is actually the bottom most plane. So it's distance should be higher
				REQUIRE( top_signed_distance > bottom_signed_distance );
			}

			std::vector< WorldCoordinate > enter_points {};
			std::vector< WorldCoordinate > exit_points {};

			WHEN( "The top plane is processed" )
			{
				processPlane( frustum.top, line, enter_points, exit_points );

				THEN( "The enter point vector should be added too" )
				{
					REQUIRE( enter_points.size() == 1 );

					AND_THEN( "The point gotten should be forward (positive distance)" )
					{
						REQUIRE( signedDistance( line.direction(), enter_points[ 0 ], line.start ) > 0.0f );
					}
				}
			}

			WHEN( "The bottom plane is processed" )
			{
				processPlane( frustum.bottom, line, enter_points, exit_points );

				THEN( "The exit point vector should be added too" )
				{
					REQUIRE( exit_points.size() == 1 );

					AND_THEN( "The point gotten should be backwards (negative distance)" )
					{
						REQUIRE( signedDistance( line.direction(), exit_points[ 0 ], line.start ) < 0.0f );
					}
				}
			}

			WHEN( "top and bottom planes are processed" )
			{
				processPlane( frustum.top, line, enter_points, exit_points );
				processPlane( frustum.bottom, line, enter_points, exit_points );

				THEN( "The enter point found should be more positive then the original point" )
				{
					const float line_start_distance { signedDistance( line.direction(), line.start, line.start ) };
					const float enter_distance { signedDistance( line.direction(), enter_points[ 0 ], line.start ) };

					REQUIRE( line_start_distance < enter_distance );
				}

				const auto top_intersection { frustum.top.intersection( line ) };
				THEN( "The last enter point should be the top plane intersection" )
				{
					REQUIRE( getLastEnter( enter_points, line ) == top_intersection );
				}

				const auto bottom_intersection { frustum.bottom.intersection( line ) };
				THEN( "The first exit point should be the bottom plane intersection" )
				{
					REQUIRE( getFirstExit( exit_points, line ) == bottom_intersection );
				}

				THEN( "The top intersection should happen after the bottom intersection" )
				{
					const float top_distance { signedDistance( line.direction(), top_intersection, line.start ) };
					const float bottom_distance { signedDistance( line.direction(), bottom_intersection, line.start ) };

					REQUIRE( top_distance > bottom_distance );
				}
			}

			THEN( "The line should NOT intersect the frustum" )
			{
				REQUIRE_FALSE( frustum.intersects( line ) );
			}
		}

		GIVEN( "A line going from UP FORWARD to DOWN FORWARD" )
		{
			const Line< CoordinateSpace::World > line { constants::WORLD_FORWARD + ( constants::WORLD_UP * 2.0f ),
				                                        constants::WORLD_FORWARD + ( constants::WORLD_DOWN * 2.0f ) };

			THEN( "The line should intersect the frustum" )
			{
				REQUIRE( frustum.intersects( line ) );
			}
		}
	}

	GIVEN( "A camera translated to (WORLD_LEFT,0,0) * 250.0f" )
	{
		camera.setView( constants::WORLD_LEFT * 250.0f, constants::DEFAULT_ROTATION );

		AND_GIVEN( "A point directly infront of the camera" )
		{
			WorldCoordinate point { constants::WORLD_LEFT * 250.0f + constants::WORLD_FORWARD };

			THEN( "The point should be in the frustum" )
			{
				const auto frustum { camera.getFrustumBounds() };

				REQUIRE( frustum.intersects( point ) );
			}
		}
	}
}