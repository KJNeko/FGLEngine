//
// Created by kj16609 on 11/28/23.
//

#include "Model.hpp"

#include <cassert>
#include <cstring>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#pragma GCC diagnostic pop

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>

#include "engine/buffers/Buffer.hpp"
#include "utils.hpp"

namespace std
{

	template <>
	struct hash< fgl::engine::Vertex >
	{
		size_t operator()( const fgl::engine::Vertex& vertex ) const
		{
			std::size_t seed { 0 };
			fgl::engine::hashCombine( seed, vertex.m_position, vertex.m_color, vertex.m_normal, vertex.m_uv );
			return seed;
		}
	};

} // namespace std

namespace fgl::engine
{

	vk::DrawIndexedIndirectCommand Model::
		buildParameters( VertexBufferSuballocation& vertex_buffer, IndexBufferSuballocation& index_buffer )
	{
		vk::DrawIndexedIndirectCommand cmd;

		cmd.indexCount = index_buffer.count();
		cmd.firstIndex = index_buffer.getOffsetCount();

		cmd.vertexOffset = static_cast< std::int32_t >( vertex_buffer.getOffsetCount() );

		cmd.firstInstance = 0;
		cmd.instanceCount = 1;

		return cmd;
	}

	Model::Model( Device& device, const Builder& builder ) :
	  m_device( device ),
	  m_vertex_buffer( builder.m_vertex_buffer, builder.verts ),
	  has_index_buffer( builder.m_index_buffer.size() > 0 ),
	  m_index_buffer( builder.m_index_buffer, builder.indicies ),
	  m_draw_parameters( buildParameters( m_vertex_buffer, m_index_buffer ) )
	{}

	std::unique_ptr< Model > Model::
		createModel( Device& device, const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer )
	{
		Builder builder { vertex_buffer, index_buffer };
		builder.loadModel( path );

		return std::make_unique< Model >( device, builder );
	}

	void Model::syncBuffers( vk::CommandBuffer& cmd_buffer )
	{
		m_vertex_buffer.stage( cmd_buffer );

		m_index_buffer.stage( cmd_buffer );
	}

	void Model::bind( vk::CommandBuffer& cmd_buffer )
	{
		std::vector< vk::Buffer > vertex_buffers { m_vertex_buffer.getVkBuffer() };

		cmd_buffer.bindVertexBuffers( 0, vertex_buffers, { 0 } );

		if ( has_index_buffer ) cmd_buffer.bindIndexBuffer( m_index_buffer.getVkBuffer(), 0, vk::IndexType::eUint32 );
	}

	void Model::draw( vk::CommandBuffer& cmd_buffer )
	{
		cmd_buffer.drawIndexed(
			m_draw_parameters.indexCount,
			m_draw_parameters.instanceCount,
			m_draw_parameters.firstIndex,
			m_draw_parameters.vertexOffset,
			m_draw_parameters.firstInstance );
	}

	std::vector< vk::VertexInputBindingDescription > Vertex::getBindingDescriptions()
	{
		std::vector< vk::VertexInputBindingDescription > binding_descriptions {
			{ 0, sizeof( Vertex ), vk::VertexInputRate::eVertex },
			{ 1, sizeof( ModelMatrixInfo ), vk::VertexInputRate::eInstance }
		};

		return binding_descriptions;
	}

	std::vector< vk::VertexInputAttributeDescription > Vertex::getAttributeDescriptions()
	{
		std::vector< vk::VertexInputAttributeDescription > attribute_descriptions {};

		attribute_descriptions.emplace_back( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, m_position ) );
		attribute_descriptions.emplace_back( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, m_color ) );
		attribute_descriptions.emplace_back( 2, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, m_normal ) );
		attribute_descriptions.emplace_back( 3, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, m_uv ) );

		//Normal Matrix
		attribute_descriptions.emplace_back( 4, 1, vk::Format::eR32G32B32A32Sfloat, 0 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 5, 1, vk::Format::eR32G32B32A32Sfloat, 1 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 6, 1, vk::Format::eR32G32B32A32Sfloat, 2 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 7, 1, vk::Format::eR32G32B32A32Sfloat, 3 * sizeof( glm::vec4 ) );

		//Normal Matrix
		attribute_descriptions.emplace_back( 8, 1, vk::Format::eR32G32B32A32Sfloat, 4 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 9, 1, vk::Format::eR32G32B32A32Sfloat, 5 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 10, 1, vk::Format::eR32G32B32A32Sfloat, 6 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 11, 1, vk::Format::eR32G32B32A32Sfloat, 7 * sizeof( glm::vec4 ) );

		static_assert( 8 * sizeof( glm::vec4 ) == sizeof( ModelMatrixInfo ) );

		return attribute_descriptions;
	}

	void Model::Builder::loadModel( const std::filesystem::path& filepath )
	{
		verts.clear();
		indicies.clear();

		tinyobj::attrib_t attrib {};
		std::vector< tinyobj::shape_t > shapes {};
		std::vector< tinyobj::material_t > materials {};
		std::string warn {};
		std::string error {};

		if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &error, filepath.string< char >().c_str() ) )
			throw std::runtime_error( warn + error );

		std::unordered_map< Vertex, std::uint32_t > unique_verts {};

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

		std::cout << unique_verts.size() << " unique verts" << std::endl;
	}
} // namespace fgl::engine