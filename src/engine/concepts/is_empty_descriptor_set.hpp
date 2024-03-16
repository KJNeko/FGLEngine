//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include "is_descriptor_set.hpp"
#include "is_empty_descriptor.hpp"

namespace fgl::engine
{

	template < typename T >
	concept is_empty_descriptor_set = is_descriptor_set< T > && ( T::descriptor_count == 1 )
	                               && is_empty_descriptor< typename T::template Binding< 0 > >;
}
