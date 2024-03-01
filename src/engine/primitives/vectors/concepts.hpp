//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include <glm/vec3.hpp>

namespace fgl::engine
{

	template < typename T > concept is_vector = requires( T t ) { requires std::is_base_of_v< glm::vec3, T >; };

	template < typename T > concept is_normal_vector = requires( T t ) { requires is_vector< T >; };

} // namespace fgl::engine
