//
// Created by kj16609 on 5/18/24.
//

#pragma once

#include <filesystem>
#include <vector>

namespace fgl::engine
{
	struct Vertex;
	struct Primitive;

	namespace memory
	{
		class Buffer;
	}

	struct ModelBuilder
	{
		memory::Buffer& m_vertex_buffer;
		memory::Buffer& m_index_buffer;

		std::vector< Primitive > m_primitives {};

		ModelBuilder() = delete;

		ModelBuilder( memory::Buffer& parent_vertex_buffer, memory::Buffer& parent_index_buffer ) :
		  m_vertex_buffer( parent_vertex_buffer ),
		  m_index_buffer( parent_index_buffer )
		{}

		void loadModel( const std::filesystem::path& filepath );
		void loadObj( const std::filesystem::path& filepath );
		void loadGltf( const std::filesystem::path& filepath );
		void loadVerts( std::vector< Vertex > verts, std::vector< std::uint32_t > indicies );
	};

} // namespace fgl::engine