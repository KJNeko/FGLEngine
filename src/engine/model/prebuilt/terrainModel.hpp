//
// Created by kj16609 on 3/16/24.
//

#pragma once

#include <memory>

namespace fgl::engine
{
	class Model;

	namespace memory
	{
		class Buffer;
	}

	std::shared_ptr< Model > generateTerrainModel( memory::Buffer& vertex_buffer, memory::Buffer& index_buffer );

} // namespace fgl::engine
