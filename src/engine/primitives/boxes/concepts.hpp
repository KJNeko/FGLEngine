//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include <concepts>

#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/concepts.hpp"
#include "engine/primitives/vectors/concepts.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	class Coordinate;

	template < typename T >
	concept has_space_type = requires( const T t ) {
		{
			T::SpaceType
		} -> std::same_as< CoordinateSpace >;
	};

	template < typename T >
	concept is_bounding_box = requires( const T t ) {
		requires has_space_type< T >;
		{
			t.right()
		} -> is_normal_vector;
		{
			t.up()
		} -> is_normal_vector;
		{
			t.forward()
		} -> is_normal_vector;
		{
			t.topLeftForward()
		} -> is_coordinate;
		{
			t.bottomLeftBack()
		} -> is_coordinate;
	};

} // namespace fgl::engine
