//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	class InfiniteLine
	{
		Coordinate< CType > m_start;
		NormalVector m_direction;

	  public:

		InfiniteLine() = delete;

		InfiniteLine( const Coordinate< CType > point, const Vector vector ) : m_start( point ), m_direction( vector )
		{}

		NormalVector getDirection() const { return m_direction; }

		Coordinate< CType > getPosition() const { return m_start; }
	};

} // namespace fgl::engine
