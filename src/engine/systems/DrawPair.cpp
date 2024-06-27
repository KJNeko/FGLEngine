//
// Created by kj16609 on 3/14/24.
//

#include "DrawPair.hpp"

#include <tracy/TracyC.h>

#include <unordered_map>

#include "engine/GameObject.hpp"
#include "engine/model/Model.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine
{

	std::pair< std::vector< vk::DrawIndexedIndirectCommand >, std::vector< ModelMatrixInfo > > getDrawCallsFromTree(
		OctTreeNode& root,
		const Frustum< CoordinateSpace::World >& frustum,
		const GameObjectFlagType flags,
		const GameObjectFilterOptions options )
	{
		ZoneScoped;
		std::unordered_map< DrawKey, DrawPair > draw_pairs {};

		const auto nodes { root.getAllLeafsInFrustum( frustum ) };

		for ( auto* node : nodes )
		{
			ZoneScopedN( "Process leaf" );
			for ( const auto& obj : *node )
			{
				ZoneScopedN( "Process object" );

				if ( !( ( obj.flags() & flags ) == flags ) ) continue;

				assert( obj.hasModel() );

				// debug::world::drawBoundingBox( obj.getBoundingBox() );

				for ( const Primitive& primitive : obj.getModel()->m_primitives )
				{
					if ( !primitive.ready() ) continue;

					//assert( primitive.m_texture );
					const ModelMatrixInfo matrix_info { .model_matrix = obj.getTransform().mat4(),
						                                .texture_idx = primitive.getAlbedoTextureID() };

					// If the textureless flag is on and we have a texture then skip the primitive.c
					if ( options & TEXTURELESS )
					{
						if ( primitive.m_textures.hasTextures() ) continue;
					}
					else
					{
						// Flag is not present
						if ( !primitive.m_textures.hasTextures() ) continue;
					}

					const auto key { std::make_pair( matrix_info.texture_idx, primitive.m_index_buffer.getOffset() ) };

					assert( primitive.m_index_buffer.size() > 0 );

					if ( auto itter = draw_pairs.find( key ); itter != draw_pairs.end() )
					{
						//Draw command for this mesh already exists. Simply add a count to it
						auto& [ itter_key, pair ] = *itter;
						auto& [ existing_cmd, model_matrix ] = pair;

						existing_cmd.instanceCount += 1;
						model_matrix.emplace_back( matrix_info );
						assert( model_matrix.size() == existing_cmd.instanceCount );
					}
					else
					{
						vk::DrawIndexedIndirectCommand cmd {};

						cmd.firstIndex = primitive.m_index_buffer.getOffsetCount();
						cmd.indexCount = primitive.m_index_buffer.size();

						cmd.vertexOffset = static_cast< int32_t >( primitive.m_vertex_buffer.getOffsetCount() );

						cmd.instanceCount = 1;

						std::vector< ModelMatrixInfo > matrix_infos {};
						matrix_infos.reserve( 128 );
						matrix_infos.emplace_back( matrix_info );
						draw_pairs.emplace( key, std::make_pair( cmd, std::move( matrix_infos ) ) );
					}
				}
			}
		}

		if ( draw_pairs.empty() )
		{
			return {};
		}

		std::vector< vk::DrawIndexedIndirectCommand > draw_commands {};
		std::vector< ModelMatrixInfo > model_matrices {};

		draw_commands.reserve( draw_pairs.size() );
		model_matrices.reserve( draw_pairs.size() * 2 );

		TracyCZoneN( filter_zone_TRACY, "Reorganize draw commands", true );
		for ( auto& [ key, pair ] : draw_pairs )
		{
			auto cmd { pair.first };
			assert( cmd != vk::DrawIndexedIndirectCommand() );
			cmd.firstInstance = static_cast< std::uint32_t >( model_matrices.size() );

			assert( cmd.instanceCount == pair.second.size() );

			assert( pair.second.size() > 0 );

			draw_commands.emplace_back( cmd );
			model_matrices.insert( model_matrices.end(), pair.second.begin(), pair.second.end() );
		}

		TracyCZoneEnd( filter_zone_TRACY );

		return { std::move( draw_commands ), std::move( model_matrices ) };
	}

} // namespace fgl::engine
