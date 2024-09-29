//
// Created by kj16609 on 2/14/24.
//

#include <catch2/catch_all.hpp>

#include "engine/assets/model/ModelVertex.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "gtest_printers.hpp"

using namespace fgl::engine;

ModelVertex createTestVert( const float x, const float y, const float z )
{
	ModelVertex vert {};
	vert.m_position = { x, y, z };

	return vert;
}

TEST_CASE( "BoundingBox", "[boundingbox]" )
{
	SECTION( "Combine test" )
	{
		std::vector< ModelVertex > model_points {};

		// Top left
		model_points.push_back( createTestVert( 1.0f, 1.0f, 1.0f ) );

		// Bottom right
		model_points.push_back( createTestVert( -1.0f, -1.0f, -1.0f ) );

		OrientedBoundingBox< CoordinateSpace::Model > model_box( generateBoundingFromVerts( model_points ) );

		THEN( "The bounding box center must be at 0,0,0" )
		{
			REQUIRE( model_box.center().vec() == glm::vec3( 0.0f, 0.0f, 0.0f ) );
		}

		model_points.clear();

		// Top left
		model_points.push_back( createTestVert( 2.0f, 1.0f, 2.0f ) );

		// Bottom right
		model_points.push_back( createTestVert( -2.0f, -1.0f, -2.0f ) );

		OrientedBoundingBox< CoordinateSpace::Model > model_box2( generateBoundingFromVerts( model_points ) );

		auto combined_box = model_box.combine( model_box2 );

		//Check that the points are correct
		//The middle point should not change
		REQUIRE( combined_box.center() == model_box.center() );
		//The scale should be the max of the two boxes
		REQUIRE( combined_box.m_transform.scale == glm::vec3( 2.0f, 1.0f, 2.0f ) );

		//Check that the points are correct

		const auto out_points { combined_box.points() };
		const auto box2_points { model_box2.points() };
		REQUIRE( out_points.size() == box2_points.size() );
		REQUIRE( out_points.size() == 8 );

		for ( std::uint32_t i = 0; i < out_points.size(); ++i )
		{
			REQUIRE( out_points[ i ] == box2_points[ i ] );
		}
	}

#ifdef FGL_ENABLE_BENCHMARKS
	BENCHMARK( "Combine bounding box" )
	{
		auto generatePoints = []()
		{
			std::vector< Coordinate< CoordinateSpace::Model > > model_points {};
			for ( int i = 0; i < 1024; ++i )
			{
				model_points
					.push_back( Coordinate< CoordinateSpace::Model >( rand() % 256, rand() % 256, rand() % 256 ) );
			}
			return model_points;
		};

		BoundingBox< CoordinateSpace::Model > model_box( generateBoundingFromPoints( generatePoints() ) );

		auto lowest_point { model_box.bottomLeftBack() };
		auto highest_point { model_box.topRightForward() };

		for ( int i = 0; i < 64; ++i )
		{
			const auto points { generatePoints() };
			const auto box { generateBoundingFromPoints( points ) };

			model_box = model_box.combine( box );

			//Check each point and set it to the lowest_point if it is lower
			for ( const auto& point : points )
			{
				lowest_point.x = std::min( lowest_point.x, point.x );
				lowest_point.y = std::min( lowest_point.y, point.y );
				lowest_point.z = std::min( lowest_point.z, point.z );
				highest_point.x = std::max( highest_point.x, point.x );
				highest_point.y = std::max( highest_point.y, point.y );
				highest_point.z = std::max( highest_point.z, point.z );
			}
		}

		return model_box;
	};
#endif
}
