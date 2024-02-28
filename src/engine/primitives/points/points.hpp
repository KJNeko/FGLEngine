//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include "Coordinate.hpp"

namespace fgl::engine
{
	template < typename T >
	concept is_coordinate = requires( std::remove_reference_t< std::remove_const_t< T > > t ) {
		{
			t.x
		} -> std::same_as< float& >;
		{
			t.y
		} -> std::same_as< float& >;
		{
			t.z
		} -> std::same_as< float& >;
		{
			t.up()
		} -> std::same_as< float& >;
		{
			t.right()
		} -> std::same_as< float& >;
		{
			t.forward()
		} -> std::same_as< float& >;
	};
}
