//
// Created by kj16609 on 5/18/24.
//

#include "ModelBuilder.hpp"

#include "engine/model/Primitive.hpp"

namespace fgl::engine
{

	void ModelBuilder::loadModel( const std::filesystem::path& filepath )
	{
		ZoneScoped;
		if ( filepath.extension() == ".obj" )
		{
			loadObj( filepath );
		}
		else if ( filepath.extension() == ".gltf" )
		{
			loadGltf( filepath );
		}
		else
			throw std::runtime_error( "Unknown model file extension" );

		//Stage
		for ( auto& prim : m_primitives )
		{
			prim.m_index_buffer.stage();
			prim.m_vertex_buffer.stage();
		}
	}

	void ModelBuilder::loadVerts( std::vector< Vertex > verts, std::vector< std::uint32_t > indicies )
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