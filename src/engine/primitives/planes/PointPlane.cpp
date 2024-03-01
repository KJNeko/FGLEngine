//
// Created by kj16609 on 2/27/24.
//

#include "PointPlane.hpp"

#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	PointPlane< CType >::PointPlane( const Coordinate< CType > pos, const Vector vec ) :
	  coordinate( pos ),
	  vector( vec )
	{}

	template < CoordinateSpace CType >
	float PointPlane< CType >::distance() const
	{
		return glm::dot( vector.vec(), coordinate.vec() );
	}

	template class PointPlane< CoordinateSpace::World >;
	template class PointPlane< CoordinateSpace::Model >;
	template class PointPlane< CoordinateSpace::Camera >;

} // namespace fgl::engine
