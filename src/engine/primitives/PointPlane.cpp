//
// Created by kj16609 on 2/27/24.
//

#include "PointPlane.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	Coordinate< CType > PointPlane< CType >::intersection( const Line< CType > line ) const
	{
		return intersection( line.start, line.direction() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > PointPlane< CType >::intersection( const Coordinate< CType > point, const Vector direction )
		const
	{
		const float line_dot { glm::dot( this->direction(), point ) };
		const float direction_dot { glm::dot( this->direction(), direction ) };

		// if the dot product of the direction of the plane and the direction of the line is zero, Then there will never be an intersection
		if ( direction_dot <= std::numeric_limits< float >::epsilon()
		     && direction_dot >= -std::numeric_limits< float >::epsilon() )
			return { Coordinate< CType >( std::numeric_limits< float >::quiet_NaN() ) };

		const float t { -( line_dot - this->distance() ) / direction_dot };

		const Coordinate< CType > intersection_point { point + ( t * direction ) };

		return intersection_point;
	}

	template < CoordinateSpace CType >
	Coordinate< CType > PointPlane< CType >::mapToPlane( const Coordinate< CType > point ) const
	{
		const float distance { distanceFrom( point ) };
		return point - Coordinate< CType >( this->direction() * distance );
	}

	template class PointPlane< CoordinateSpace::World >;
	template class PointPlane< CoordinateSpace::Model >;
	template class PointPlane< CoordinateSpace::Camera >;

} // namespace fgl::engine
