//
// Created by kj16609 on 11/28/23.
//

#include "Model.hpp"

#include <cassert>
#include <iostream>

#include "builders/ModelBuilder.hpp"
#include "builders/SceneBuilder.hpp"
#include "engine/buffers/Buffer.hpp"
#include "engine/image/ImageView.hpp"

namespace fgl::engine
{

	std::vector< vk::DrawIndexedIndirectCommand > Model::buildParameters( const std::vector< Primitive >& primitives )
	{
		ZoneScoped;
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
		ZoneScoped;
		assert( primitives.size() > 0 );
		if ( primitives.size() <= 0 ) return {};

		OrientedBoundingBox< CoordinateSpace::Model > box { primitives.at( 0 ).m_bounding_box };

		for ( std::uint64_t i = 1; i < primitives.size(); i++ ) box = box.combine( primitives[ i ].m_bounding_box );

		return box;
	}

	bool Model::ready()
	{
		//Return true if even a single primitive is ready
		for ( auto& primitive : this->m_primitives )
		{
			if ( primitive.ready() ) return true;
		}
		return false;
	}

	std::vector< vk::DrawIndexedIndirectCommand > Model::getDrawCommand( const std::uint32_t index ) const
	{
		ZoneScoped;
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
		ModelBuilder& builder, const OrientedBoundingBox< CoordinateSpace::Model > bounding_box, std::string name ) :
	  Model( std::move( builder.m_primitives ), bounding_box, name )
	{}

	Model::Model(
		std::vector< Primitive >&& primitives,
		const OrientedBoundingBox< CoordinateSpace::Model > bounding_box,
		std::string name ) :
	  m_draw_parameters( buildParameters( primitives ) ),
	  m_name( name ),
	  m_bounding_box( bounding_box )
	{
		assert( bounding_box.middle.vec() != constants::DEFAULT_VEC3 );
		m_primitives = std::move( primitives );
	}

	std::shared_ptr< Model > Model::
		createModel( const std::filesystem::path& path, memory::Buffer& vertex_buffer, memory::Buffer& index_buffer )
	{
		ZoneScoped;
		std::cout << "Creating model: " << path << std::endl;
		ModelBuilder builder { vertex_buffer, index_buffer };
		builder.loadModel( path );

		//Calculate bounding box
		OrientedBoundingBox bounding_box { buildBoundingBox( builder.m_primitives ) };

		auto model_ptr { std::make_shared< Model >( builder, bounding_box ) };

		std::cout << "Finished making model: " << path << std::endl;
		return model_ptr;
	}

	std::shared_ptr< Model > Model::createModelFromVerts(
		std::vector< ModelVertex > verts,
		std::vector< std::uint32_t > indicies,
		memory::Buffer& vertex_buffer,
		memory::Buffer& index_buffer )
	{
		ZoneScoped;
		ModelBuilder builder { vertex_buffer, index_buffer };
		builder.loadVerts( std::move( verts ), std::move( indicies ) );

		OrientedBoundingBox bounding_box { buildBoundingBox( builder.m_primitives ) };

		auto model_ptr { std::make_shared< Model >( builder, bounding_box ) };

		return model_ptr;
	}

} // namespace fgl::engine