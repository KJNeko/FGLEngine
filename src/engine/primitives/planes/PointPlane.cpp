//
// Created by kj16609 on 2/27/24.
//

#include "PointPlane.hpp"

#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	PointPlane< CType >::PointPlane() :
	  coordinate( constants::WORLD_CENTER ),
	  vector( NormalVector( constants::WORLD_Y ) )
	{}

	template < CoordinateSpace CType >
	PointPlane< CType >::PointPlane( const Coordinate< CType > pos, const Vector vec ) :
	  coordinate( pos ),
	  vector( vec )
	{}

	template < CoordinateSpace CType >
	PointPlane< CType >::PointPlane( const Coordinate< CType > pos, const NormalVector vec ) :
	  coordinate( pos ),
	  vector( vec )
	{}

	template < CoordinateSpace CType >
	float PointPlane< CType >::distance() const
	{
		return glm::dot( vector.vec(), coordinate.vec() );
	}

	template < CoordinateSpace CType >
	float PointPlane< CType >::distanceFrom( const Coordinate< CType > coord ) const
	{
		return static_cast< float >( glm::dot( ( coord - coordinate ).vec(), vector.vec() ) );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > PointPlane< CType >::mapToPlane( const Coordinate< CType > point ) const
	{
		const float distance { distanceFrom( point ) };

		return point - ( this->getDirection() * distance );
	}

	template class PointPlane< CoordinateSpace::World >;
	template class PointPlane< CoordinateSpace::Model >;
	template class PointPlane< CoordinateSpace::Screen >;

} // namespace fgl::engine
