//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include "Coordinate.hpp"
#include "Vector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct Line
	{
		Coordinate< CType > start;
		Coordinate< CType > end;

		Vector direction() const { return Vector( static_cast< glm::vec3 >( end - start ) ); }

		Line( const Coordinate< CType > i_start, const Coordinate< CType > i_end ) noexcept :
		  start( i_start ),
		  end( i_end )
		{}

		Line( const glm::vec3 i_start, glm::vec3 i_end ) : start( i_start ), end( i_end ) {}

		inline Line< CType > flip() const { return { end, start }; }
	};

	template < CoordinateSpace CType, MatrixType MType >
	Line< EvolvedType< CType >() > operator*( const Matrix< MType > mat, const Line< CType > line )
	{
		return { mat * line.start, mat * line.end };
	}

} // namespace fgl::engine
