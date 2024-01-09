//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include <type_traits>

#include "engine/buffers/Buffer.hpp"

namespace fgl::engine
{
	struct BufferSuballocationInfo;

	template < typename T > concept is_buffer = std::same_as< T, Buffer >;

	template < typename T > concept is_buffer_ref = is_buffer< std::remove_reference_t< T > >;

} // namespace fgl::engine