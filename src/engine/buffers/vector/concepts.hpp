//
// Created by kj16609 on 6/26/24.
//

#pragma once

#include "engine/buffers/BufferSuballocation.hpp"

namespace fgl::engine::memory
{
	struct DeviceVectorBase
	{};

	template < typename T >
	concept is_device_vector = requires( T t ) {
		requires std::is_base_of_v< DeviceVectorBase, T >;
		requires std::is_base_of_v< BufferSuballocation, T >;
	};

} // namespace fgl::engine::memory
