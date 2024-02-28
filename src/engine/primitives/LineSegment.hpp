//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include "Coordinate.hpp"
#include "Vector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct LineSegment
	{
		Coordinate< CType > start;
		Coordinate< CType > end;

		Vector direction() const { return Vector( static_cast< glm::vec3 >( end - start ) ); }

		LineSegment( const Coordinate< CType > i_start, const Coordinate< CType > i_end ) noexcept :
		  start( i_start ),
		  end( i_end )
		{}

		LineSegment( const glm::vec3 i_start, glm::vec3 i_end ) : start( i_start ), end( i_end ) {}

		inline LineSegment< CType > flip() const { return { end, start }; }
	};

	template < CoordinateSpace CType, MatrixType MType >
	LineSegment< EvolvedType< CType >() > operator*( const Matrix< MType > mat, const LineSegment< CType > line )
	{
		return { mat * line.start, mat * line.end };
	}

	template < CoordinateSpace CType >
	using Line = LineSegment< CType >;

} // namespace fgl::engine
