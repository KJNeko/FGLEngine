//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include "NormalVector.hpp"
#include "Vector.hpp"

namespace fgl::engine
{
	template < typename T > concept is_normal_vector = std::same_as< T, NormalVector >;
	template < typename T > concept is_vector = std::same_as< T, Vector > || is_normal_vector< T >;
} // namespace fgl::engine