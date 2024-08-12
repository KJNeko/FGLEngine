//
// Created by kj16609 on 3/14/24.
//

#include "DrawPair.hpp"

#include <tracy/TracyC.h>

#include <unordered_map>

#include "engine/debug/drawers.hpp"
#include "engine/gameobjects/components/ModelComponent.hpp"
#include "engine/model/Model.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine
{

	std::pair< std::vector< vk::DrawIndexedIndirectCommand >, std::vector< ModelMatrixInfo > > getDrawCallsFromTree(
		OctTreeNode& root,
		const Frustum< CoordinateSpace::World >& frustum,
		const GameObjectFlagType game_object_flags,
		const TreeFilterFlags tree_flags,
		std::function< bool( const GameObject& ) > filterFunc )
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

				if ( ( obj.flags() & game_object_flags ) != game_object_flags ) continue;
				if ( !filterFunc( obj ) ) continue;

				//Check if we have a renderable component
				if ( !obj.hasComponent< ModelComponent >() ) continue;

				const auto model_components { obj.getComponents< ModelComponent >() };

				for ( const auto* model_component_ptr : model_components )
				{
					const auto& model_transform { model_component_ptr->m_transform };

					const auto& comp { *model_component_ptr };
					for ( const Primitive& primitive : comp->m_primitives )
					{
						if ( !primitive.ready() ) continue;

						const Matrix< MatrixType::ModelToWorld > matrix { obj.getTransform().mat()
							                                              * model_transform.mat() };

						// Does this primitive pass the bounds check
						if ( !frustum.intersects( matrix * primitive.getBoundingBox() ) ) continue;

						//assert( primitive.m_texture );
						const ModelMatrixInfo matrix_info { .model_matrix = matrix,
							                                .albedo_id = primitive.getAlbedoTextureID(),
							                                .normal_id = primitive.getNormalTextureID() };

						// If the textureless flag is on and we have a texture then skip the primitive.c
						if ( tree_flags & IS_TEXTURELESS )
						{
							if ( primitive.m_textures.hasTextures() ) continue;
						}
						else
						{
							// Flag is not present
							if ( !primitive.m_textures.hasTextures() ) continue;
						}

						const auto key {
							std::make_pair( matrix_info.albedo_id, primitive.m_index_buffer.getOffset() )
						};

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
