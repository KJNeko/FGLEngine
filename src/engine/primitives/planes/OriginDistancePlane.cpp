//
// Created by kj16609 on 2/16/24.
//

#include "OriginDistancePlane.hpp"

#include "engine/primitives/lines/InfiniteLine.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	OriginDistancePlane< CType >::OriginDistancePlane() :
	  m_distance( std::numeric_limits< float >::infinity() ),
	  m_direction( constants::WORLD_FORWARD )
	{}

	template < CoordinateSpace CType >
	OriginDistancePlane< CType >::OriginDistancePlane( const NormalVector vector, const float distance ) :
	  m_distance( distance ),
	  m_direction( vector )
	{}

	template < CoordinateSpace CType >
	OriginDistancePlane< CType >::OriginDistancePlane( const Vector vector, const float distance ) :
	  m_distance( distance ),
	  m_direction( vector )
	{}

	template < CoordinateSpace CType >
	Coordinate< CType > OriginDistancePlane< CType >::getPosition() const
	{
		return Coordinate< CType >( m_direction * m_distance );
	}

	template < CoordinateSpace CType >
	float OriginDistancePlane< CType >::distanceFrom( const Coordinate< CType > coord ) const
	{
		return glm::dot( m_direction.vec(), coord.vec() ) - m_distance;
	}

	template < CoordinateSpace CType >
	Coordinate< CType > OriginDistancePlane< CType >::mapToPlane( const Coordinate< CType > point ) const
	{
		const float distance { this->distanceFrom( point ) };

		return point - ( this->m_direction * distance );
	}

	template class OriginDistancePlane< CoordinateSpace::Model >;
	template class OriginDistancePlane< CoordinateSpace::World >;
	template class OriginDistancePlane< CoordinateSpace::Screen >;

} // namespace fgl::engine
