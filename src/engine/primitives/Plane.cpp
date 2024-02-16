//
// Created by kj16609 on 2/16/24.
//

#include "Plane.hpp"

namespace fgl::engine
{

	template <>
	double Plane< CoordinateSpace::World >::distanceFrom( const WorldCoordinate coord ) const
	{
		return glm::dot( m_direction, coord ) - m_distance;
	}

} // namespace fgl::engine
