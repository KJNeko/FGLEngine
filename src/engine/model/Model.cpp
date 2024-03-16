//
// Created by kj16609 on 11/28/23.
//

#include "Model.hpp"

#include <cassert>
#include <cstring>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>

#include "engine/buffers/Buffer.hpp"
#include "engine/buffers/SuballocationView.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/image/Sampler.hpp"

namespace fgl::engine
{

	std::vector< vk::DrawIndexedIndirectCommand > Model::buildParameters( const std::vector< Primitive >& primitives )
	{
		std::vector< vk::DrawIndexedIndirectCommand > draw_parameters {};
		draw_parameters.reserve( primitives.size() );

		for ( const auto& primitive : primitives )
		{
			vk::DrawIndexedIndirectCommand cmd;
			cmd.indexCount = primitive.m_index_buffer.size();
			cmd.firstIndex = primitive.m_index_buffer.getOffsetCount();

			cmd.vertexOffset = static_cast< std::int32_t >( primitive.m_vertex_buffer.getOffsetCount() );

			cmd.firstInstance = 0;
			cmd.instanceCount = 1;

			draw_parameters.emplace_back( std::move( cmd ) );
		}

		return draw_parameters;
	}

	OrientedBoundingBox< CoordinateSpace::Model > Model::buildBoundingBox( const std::vector< Primitive >& primitives )
	{
		assert( primitives.size() > 0 );
		if ( primitives.size() <= 0 ) return {};

		OrientedBoundingBox< CoordinateSpace::Model > box { primitives.at( 0 ).m_bounding_box };

		for ( std::uint64_t i = 1; i < primitives.size(); i++ ) box = box.combine( primitives[ i ].m_bounding_box );

		return box;
	}

	std::vector< vk::DrawIndexedIndirectCommand > Model::getDrawCommand( const std::uint32_t index ) const
	{
		std::vector< vk::DrawIndexedIndirectCommand > draw_commands {};
		draw_commands.reserve( m_primitives.size() );
		for ( const auto& cmd : m_draw_parameters )
		{
			auto new_cmd { cmd };
			new_cmd.firstInstance = index;

			draw_commands.push_back( new_cmd );
		}

		return draw_commands;
	}

	Model::Model(
		Device& device, ModelBuilder& builder, const OrientedBoundingBox< CoordinateSpace::Model > bounding_box ) :
	  m_device( device ),
	  m_draw_parameters( buildParameters( builder.m_primitives ) ),
	  m_bounding_box( bounding_box )
	{
		assert( bounding_box.middle.vec() != constants::DEFAULT_VEC3 );
		m_primitives = std::move( builder.m_primitives );
	}

	std::unique_ptr< Model > Model::
		createModel( Device& device, const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer )
	{
		std::cout << "Creating model: " << path << std::endl;
		ModelBuilder builder { vertex_buffer, index_buffer };
		builder.loadModel( path );

		//Calculate bounding box
		OrientedBoundingBox bounding_box { buildBoundingBox( builder.m_primitives ) };

		auto model_ptr { std::make_unique< Model >( device, builder, bounding_box ) };

		std::cout << "Finished making model: " << path << std::endl;
		return model_ptr;
	}

	std::unique_ptr< Model > Model::createModelFromVerts(
		Device& device,
		std::vector< Vertex > verts,
		std::vector< std::uint32_t > indicies,
		Buffer& vertex_buffer,
		Buffer& index_buffer )
	{
		ModelBuilder builder { vertex_buffer, index_buffer };
		builder.loadVerts( std::move( verts ), std::move( indicies ) );

		OrientedBoundingBox bounding_box { buildBoundingBox( builder.m_primitives ) };

		auto model_ptr { std::make_unique< Model >( device, builder, bounding_box ) };

		return model_ptr;
	}

	void Model::syncBuffers( vk::CommandBuffer& cmd_buffer )
	{
		for ( auto& primitive : m_primitives )
		{
			primitive.m_vertex_buffer.stage( cmd_buffer );
			primitive.m_index_buffer.stage( cmd_buffer );

			if ( primitive.m_texture.has_value() )
			{
				primitive.m_texture->stage( cmd_buffer );
			}
		}
	}

	void ModelBuilder::loadModel( const std::filesystem::path& filepath )
	{
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
	}

	void ModelBuilder::loadVerts( std::vector< Vertex > verts, std::vector< std::uint32_t > indicies )
	{
		VertexBufferSuballocation vertex_suballoc { this->m_vertex_buffer, verts };
		IndexBufferSuballocation index_suballoc { this->m_index_buffer, std::move( indicies ) };

		this->m_primitives.emplace_back(
			std::move( vertex_suballoc ), std::move( index_suballoc ), generateBoundingFromVerts( verts ) );
	}

} // namespace fgl::engine