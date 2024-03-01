//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include <concepts>

namespace fgl::engine
{
	template < typename T >
	concept is_coordinate = requires( T t ) {
		requires std::is_base_of_v< glm::vec3, T >;
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
