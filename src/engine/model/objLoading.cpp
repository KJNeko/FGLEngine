//
// Created by kj16609 on 2/5/24.
//

#include "Model.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "objectloaders/tiny_obj_loader.h"
#pragma GCC diagnostic pop

#include "Vertex.hpp"

namespace fgl::engine
{

	void ModelBuilder::loadObj( const std::filesystem::path& filepath )
	{
		m_primitives.clear();

		tinyobj::attrib_t attrib {};
		std::vector< tinyobj::shape_t > shapes {};
		std::vector< tinyobj::material_t > materials {};
		std::string warn {};
		std::string error {};

		if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &error, filepath.string< char >().c_str() ) )
			throw std::runtime_error( warn + error );

		std::unordered_map< Vertex, std::uint32_t > unique_verts {};

		std::vector< Vertex > verts;
		std::vector< std::uint32_t > indicies;

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

					assert( vert.m_color[ 0 ] > 0.2f );
					assert( vert.m_color[ 1 ] > 0.2f );
					assert( vert.m_color[ 2 ] > 0.2f );
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

		std::vector< ModelCoordinate > vert_pos;
		for ( const auto& vert : verts )
		{
			vert_pos.emplace_back( vert.m_position );
		}

		BoundingBox bounding_box { generateBoundingFromPoints( vert_pos ) };

		m_primitives.emplace_back(
			VertexBufferSuballocation( m_vertex_buffer, std::move( verts ) ),
			IndexBufferSuballocation( m_index_buffer, std::move( indicies ) ),
			bounding_box );

		std::cout << unique_verts.size() << " unique verts" << std::endl;
	}
} // namespace fgl::engine
