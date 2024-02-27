//
// Created by kj16609 on 2/16/24.
//

#include "OriginDistancePlane.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Line.hpp"

namespace fgl::engine
{

	template <>
	float OriginDistancePlane< CoordinateSpace::World >::distanceFrom( const WorldCoordinate coord ) const
	{
		return glm::dot( m_direction, coord ) - m_distance;
	}

	template <>
	bool OriginDistancePlane< CoordinateSpace::World >::intersects( const Line< CoordinateSpace::World > line ) const
	{
		return isForward( line.start ) != isForward( line.end );
	}

	template <>
	Coordinate< CoordinateSpace::World > OriginDistancePlane<
		CoordinateSpace::World >::intersection( const Line< CoordinateSpace::World > line ) const
	{
		const WorldCoordinate line_start { line.start };
		const Vector direction { line.direction() };

		return intersection( line.start, line.direction() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > OriginDistancePlane<
		CType >::intersection( const Coordinate< CType > point, const Vector direction ) const
	{
		const float line_dot { glm::dot( this->direction(), point ) };
		const float direction_dot { glm::dot( this->direction(), direction ) };

		// if the dot product of the direction of the plane and the direction of the line is zero, Then there will never be an intersection
		if ( direction_dot <= std::numeric_limits< float >::epsilon()
		     && direction_dot >= -std::numeric_limits< float >::epsilon() )
			return { WorldCoordinate( std::numeric_limits< float >::quiet_NaN() ) };

		const float t { -( line_dot - this->distance() ) / direction_dot };

		const WorldCoordinate intersection_point { point + ( t * direction ) };

		return intersection_point;
	}

	template < CoordinateSpace CType >
	Coordinate< CType > OriginDistancePlane< CType >::mapToPlane( const Coordinate< CType > point ) const
	{
		const float distance { distanceFrom( point ) };

		return point - ( this->m_direction * distance );
	}

} // namespace fgl::engine
