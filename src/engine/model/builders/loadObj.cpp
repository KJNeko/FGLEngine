//
// Created by kj16609 on 5/18/24.
//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include <objectloaders/tiny_obj_loader.h>
#pragma GCC diagnostic pop

#include <fstream>
#include <unordered_map>

#include "ModelBuilder.hpp"
#include "engine/model/Primitive.hpp"
#include "engine/model/Vertex.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"

namespace fgl::engine
{
	void ModelBuilder::loadObj( const std::filesystem::path& filepath )
	{
		assert( std::filesystem::exists( filepath ) );
		m_primitives.clear();

		tinyobj::ObjReader reader {};
		reader.ParseFromFile( filepath.string() );

		if ( !reader.Valid() ) throw std::runtime_error( "Reader not valid" );

		const tinyobj::attrib_t& attrib { reader.GetAttrib() };
		const std::vector< tinyobj::shape_t >& shapes { reader.GetShapes() };
		const std::vector< tinyobj::material_t >& materials { reader.GetMaterials() };

		if ( shapes.size() == 0 ) throw std::runtime_error( "Failed to get shapes from OBJ" );

		const std::string& warn { reader.Warning() };
		const std::string& error { reader.Error() };

		if ( !warn.empty() ) log::warn( "While loading model {}: {}", filepath, warn );

		if ( !error.empty() ) log::error( "While loading model {}: {}", filepath, error );

		std::unordered_map< Vertex, std::uint32_t > unique_verts {};

		std::vector< Vertex > verts {};
		std::vector< std::uint32_t > indicies {};

		for ( const auto& shape : shapes )
		{
			for ( const auto& index : shape.mesh.indices )
			{
				Vertex vert {};
				if ( index.vertex_index >= 0 )
				{
					vert.m_position = {
						attrib.vertices[ static_cast< std::uint64_t >( 3 * index.vertex_index + 0 ) ],
						attrib.vertices[ static_cast< std::uint64_t >( 3 * index.vertex_index + 1 ) ],
						attrib.vertices[ static_cast< std::uint64_t >( 3 * index.vertex_index + 2 ) ],
					};

					vert.m_color = { attrib.colors[ static_cast< std::uint64_t >( 3 * index.vertex_index + 0 ) ],
						             attrib.colors[ static_cast< std::uint64_t >( 3 * index.vertex_index + 1 ) ],
						             attrib.colors[ static_cast< std::uint64_t >( 3 * index.vertex_index + 2 ) ] };
				}

				if ( index.normal_index >= 0 )
				{
					vert.m_normal = {
						attrib.normals[ static_cast< std::uint64_t >( 3 * index.normal_index + 0 ) ],
						attrib.normals[ static_cast< std::uint64_t >( 3 * index.normal_index + 1 ) ],
						attrib.normals[ static_cast< std::uint64_t >( 3 * index.normal_index + 2 ) ],
					};
				}

				if ( index.texcoord_index >= 0 )
				{
					vert.m_uv = {
						attrib.texcoords[ static_cast< std::uint64_t >( 3 * index.texcoord_index + 0 ) ],
						attrib.texcoords[ static_cast< std::uint64_t >( 3 * index.texcoord_index + 1 ) ],
					};
				}

				if ( auto itter = unique_verts.find( vert ); itter != unique_verts.end() )
				{
					indicies.push_back( unique_verts[ vert ] );
				}
				else
				{
					const auto insert_op { unique_verts.insert( std::make_pair( vert, verts.size() ) ) };
					verts.emplace_back( vert );

					if ( insert_op.second )
						indicies.push_back( insert_op.first->second );
					else
						throw std::runtime_error( "Failed to insert new vertex" );
				}
			}
		}

		for ( [[maybe_unused]] const auto idx : indicies )
		{
			assert( idx < verts.size() );
		}

		const OrientedBoundingBox bounding_box { generateBoundingFromVerts( verts ) };

		[[maybe_unused]] auto& itter = m_primitives.emplace_back(
			VertexBufferSuballocation( m_vertex_buffer, std::move( verts ) ),
			IndexBufferSuballocation( m_index_buffer, std::move( indicies ) ),
			bounding_box,
			PrimitiveMode::TRIS );

		log::debug( "{} unique verts loading model {}", unique_verts.size(), filepath );
	}
} // namespace fgl::engine
