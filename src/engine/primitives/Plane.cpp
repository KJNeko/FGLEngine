//
// Created by kj16609 on 2/16/24.
//

#include "Plane.hpp"

#include "Line.hpp"

namespace fgl::engine
{

	template <>
	double Plane< CoordinateSpace::World >::distanceFrom( const WorldCoordinate coord ) const
	{
		return glm::dot( m_direction, coord ) - m_distance;
	}

	template <>
	bool OriginDistancePlane< CoordinateSpace::World >::intersects( const Line< CoordinateSpace::World > line ) const
	{
		return isBehind( line.start ) != isBehind( line.end );
	}

} // namespace fgl::engine
