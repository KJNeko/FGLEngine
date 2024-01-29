//
// Created by kj16609 on 1/27/24.
//

#include "WorldCoordinate.hpp"

namespace fgl::engine
{

	double Plane::distanceFrom( const Coordinate& coord ) const
	{
		return -( glm::dot( coord.position, m_direction ) - m_distance );
	}

	bool Plane::isForward( const Coordinate& coord ) const
	{
		return distanceFrom( coord ) > 0.0;
	}

	bool Plane::isBehind( const Coordinate& coord ) const
	{
		return !isForward( coord );
	}

	Plane Plane::operator*( glm::mat4 matrix ) const
	{
		assert( m_distance != std::numeric_limits< float >::max() );
		assert( m_direction != DEFAULT_COORDINATE_VEC3 );

		Plane result = *this;
		const glm::vec3 new_direction { matrix * glm::vec4( m_direction, 1.0f ) };

		const auto new_distance { glm::dot( new_direction, m_direction ) + m_distance };
		result.m_direction = glm::normalize( new_direction );
		result.m_distance = new_distance;

		return result;
	}

	bool Frustum::pointInside( const Coordinate& coord ) const
	{
		// clang-format off
		return
			near.isForward( coord ) && far.isForward( coord )
			&& top.isForward( coord ) && bottom.isForward( coord )
			&& right.isForward( coord ) && left.isForward( coord );
		// clang-format on
	}
} // namespace fgl::engine
