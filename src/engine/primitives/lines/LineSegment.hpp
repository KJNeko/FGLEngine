//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	class LineSegment
	{
		Coordinate< CType > start;
		Coordinate< CType > end;

	  public:

		NormalVector getDirection() const { return NormalVector( static_cast< glm::vec3 >( end - start ) ); }

		Coordinate< CType > getPosition() const { return start; }

		Coordinate< CType > getEnd() const { return end; }

		explicit LineSegment( const Coordinate< CType > i_start, const Coordinate< CType > i_end ) noexcept :
		  start( i_start ),
		  end( i_end )
		{}

		explicit LineSegment( const glm::vec3 i_start, glm::vec3 i_end ) : start( i_start ), end( i_end ) {}

		inline LineSegment flip() const { return LineSegment( end, start ); }
	};

	template < CoordinateSpace CType >
	using Line = LineSegment< CType >;

} // namespace fgl::engine
