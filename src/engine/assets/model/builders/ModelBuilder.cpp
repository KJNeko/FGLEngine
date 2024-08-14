//
// Created by kj16609 on 5/18/24.
//

#include "ModelBuilder.hpp"

#include "engine/assets/model/Primitive.hpp"

namespace fgl::engine
{

	void ModelBuilder::loadModel( const std::filesystem::path& filepath )
	{
		ZoneScoped;
		if ( filepath.extension() == ".obj" )
		{
			loadObj( filepath );
		}
		else
			throw std::runtime_error( "Unknown model file extension" );
	}

	void ModelBuilder::loadVerts( std::vector< ModelVertex > verts, std::vector< std::uint32_t > indicies )
	{
		ZoneScoped;
		VertexBufferSuballocation vertex_suballoc { this->m_vertex_buffer, verts };
		IndexBufferSuballocation index_suballoc { this->m_index_buffer, std::move( indicies ) };

		this->m_primitives.emplace_back(
			std::move( vertex_suballoc ),
			std::move( index_suballoc ),
			generateBoundingFromVerts( verts ),
			PrimitiveMode::TRIS );
	}

} // namespace fgl::engine