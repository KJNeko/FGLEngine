//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include "is_constant_range.hpp"
#include "is_descriptor_set.hpp"

namespace fgl::engine
{
	template < typename T > concept is_valid_pipeline_input = is_constant_range< T > || is_descriptor_set< T >;
}