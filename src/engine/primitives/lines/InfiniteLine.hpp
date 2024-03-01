//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	class InfiniteLine
	{
		Coordinate< CType > m_start;
		NormalVector m_direction;

		Coordinate< CType > intersection( const Coordinate< CType > position, const NormalVector vector );

	  public:

		InfiniteLine() = delete;

		InfiniteLine( const Coordinate< CType > point, const NormalVector vector ) :
		  m_start( point ),
		  m_direction( vector )
		{}

		NormalVector getDirection() const { return m_direction; }

		Coordinate< CType > getPosition() const { return m_start; }

		InfiniteLine operator-() const { return InfiniteLine( m_start, -m_direction ); }

		inline InfiniteLine flip() const { return -( *this ); }

		bool intersects( const PointPlane< CType > t ) const;
		Coordinate< CType > intersection( const PointPlane< CType > t ) const;

		bool intersects( const OriginDistancePlane< CType > t ) const;
		Coordinate< CType > intersection( const OriginDistancePlane< CType > t ) const;
	};

} // namespace fgl::engine
