//
// Created by kj16609 on 5/18/24.
//

#pragma once

#include <filesystem>
#include <vector>

#include "engine/primitives/Transform.hpp"
#include "memory/buffers/BufferHandle.hpp"

namespace fgl::engine
{
	struct ModelVertex;
	struct Primitive;

	struct ModelBuilder
	{
		memory::Buffer m_vertex_buffer;
		memory::Buffer m_index_buffer;

		std::vector< Primitive > m_primitives {};

		ModelBuilder() = delete;

		ModelBuilder(
			const memory::Buffer& parent_vertex_buffer, const memory::Buffer& parent_index_buffer ) :
		  m_vertex_buffer( parent_vertex_buffer ),
		  m_index_buffer( parent_index_buffer )
		{}

		void loadModel( const std::filesystem::path& filepath );
		void loadObj( const std::filesystem::path& filepath );
		void loadGltf( const std::filesystem::path& filepath );
		void loadVerts( const std::vector< ModelVertex >& verts, const std::vector< std::uint32_t >& indicies );
	};

} // namespace fgl::engine