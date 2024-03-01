//
// Created by kj16609 on 2/28/24.
//

#include "Coordinate.hpp"

#include "engine/primitives/planes/concepts.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	Coordinate< CType >::Coordinate( const Vector vector ) : glm::vec3( vector.vec() )
	{}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator+( const Vector other ) const
	{
		return Coordinate< CType >( vec() + other.vec() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator-( const Vector other ) const
	{
		return Coordinate< CType >( vec() - other.vec() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType >& Coordinate< CType >::operator+=( const Vector other )
	{
		this->vec() += other.vec();
		return *this;
	}

	template < CoordinateSpace CType >
	Coordinate< CType >& Coordinate< CType >::operator-=( const Vector other )
	{
		this->vec() -= other.vec();
		return *this;
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator+( const NormalVector other ) const
	{
		return Coordinate< CType >( vec() + other.vec() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator-( const NormalVector other ) const
	{
		return Coordinate< CType >( vec() - other.vec() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator+( const Coordinate other ) const
	{
		return Coordinate< CType >( vec() + other.vec() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator-( const Coordinate other ) const
	{
		return Coordinate< CType >( vec() - other.vec() );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator+( const glm::vec3 other ) const
	{
		return Coordinate< CType >( vec() + other );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > Coordinate< CType >::operator-( const glm::vec3 other ) const
	{
		return Coordinate< CType >( vec() - other );
	}

	template class Coordinate< CoordinateSpace::Model >;
	template class Coordinate< CoordinateSpace::World >;
	template class Coordinate< CoordinateSpace::Screen >;

} // namespace fgl::engine
