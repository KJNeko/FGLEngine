//
// Created by kj16609 on 8/12/24.
//

#include "drawers.hpp"

#include <array>

#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "engine/primitives/lines/LineSegment.hpp"

namespace fgl::engine::debug
{

	void drawLine( const LineSegment< CoordinateSpace::World >& line, const glm::vec3 color )
	{
		drawLine( line.getStart(), line.getEnd(), color );
	}

	void drawBoundingBox( const AxisAlignedBoundingBox< CoordinateSpace::World >& bounding_box )
	{
		drawBoundingBox( bounding_box, glm::vec3( 1.0f ) );
	}

	void drawBoundingBox( const AxisAlignedBoundingBox< CoordinateSpace::World >& bounding_box, const glm::vec3 color )
	{
		for ( const auto& line : bounding_box.lines() )
		{
			drawLine( line.getStart(), line.getEnd(), color );
		}
	}

	void drawBoundingBox( const OrientedBoundingBox< CoordinateSpace::World >& bounding_box )
	{
		drawBoundingBox( bounding_box, glm::vec3( 1.0f ) );
	}

	void drawBoundingBox( const OrientedBoundingBox< CoordinateSpace::World >& bounding_box, const glm::vec3 color )
	{
		for ( const auto& line : bounding_box.lines() )
		{
			drawLine( line.getStart(), line.getEnd(), color );
		}
	}

	void drawAxisHelper()
	{
		constexpr WorldCoordinate center { constants::WORLD_CENTER };
		constexpr WorldCoordinate right { constants::WORLD_Y };
		constexpr WorldCoordinate forward { constants::WORLD_X };
		constexpr WorldCoordinate up { constants::WORLD_Z };

		drawLine( center, right, right.vec() );
		drawLine( center, up, up.vec() );
		drawLine( center, forward, forward.vec() );
	}

} // namespace fgl::engine::debug
