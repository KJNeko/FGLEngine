//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include <concepts>

#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/concepts.hpp"
#include "engine/primitives/vectors/concepts.hpp"

namespace fgl::engine
{

	template < CoordinateSpace >
	class Coordinate;

	class Vector;
	struct NormalVector;

	template < typename T >
	concept is_plane = requires( T t ) {
		{
			T::SpaceType
		} -> std::same_as< const CoordinateSpace& >;
		{
			t.distance()
		} -> std::same_as< float >;
		{
			t.getPosition()
		} -> is_coordinate;
		{
			t.getDirection()
		} -> is_normal_vector;
		{
			t.isForward( std::declval< Coordinate< T::SpaceType > >() )
		} -> std::same_as< bool >;
		{
			t.mapToPlane( std::declval< Coordinate< T::SpaceType > >() )
		} -> is_coordinate;
	};

} // namespace fgl::engine