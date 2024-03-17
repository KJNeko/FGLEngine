//
// Created by kj16609 on 3/16/24.
//

#pragma once

#include <memory>

namespace fgl::engine
{
	class Model;

	class Buffer;

	std::shared_ptr< Model > generateTerrainModel( Buffer& vertex_buffer, Buffer& index_buffer );

} // namespace fgl::engine
