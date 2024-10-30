//
// Created by kj16609 on 3/14/24.
//

#include "DrawPair.hpp"

#include <tracy/TracyC.h>

#include <unordered_map>

#include "engine/assets/model/Model.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/debug/profiling/counters.hpp"
#include "engine/gameobjects/components/ModelComponent.hpp"
#include "engine/math/intersections.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine
{

	std::pair< std::vector< vk::DrawIndexedIndirectCommand >, std::vector< ModelMatrixInfo > > getDrawCallsFromTree(
		OctTreeNode& root,
		const Frustum& frustum,
		const GameObjectFlagType game_object_flags,
		const TreeFilterFlags tree_flags,
		std::function< bool( const GameObject& ) > filterFunc )
	{
		ZoneScoped;
		std::unordered_map< DrawKey, DrawPair > draw_pairs {};
		draw_pairs.reserve( 512 );

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

				const Matrix< MatrixType::ModelToWorld > obj_matrix { obj.getTransform().mat() };

				for ( const auto* model_component_ptr : model_components )
				{
					const auto& model_transform { model_component_ptr->m_transform };

					const Matrix< MatrixType::ModelToWorld > world_matrix { model_transform.mat() * obj_matrix };

					const auto& comp { *model_component_ptr };
					for ( const Primitive& primitive : comp->m_primitives )
					{
						if ( !primitive.ready() ) continue;

						// Does this primitive pass the bounds check
						const OrientedBoundingBox< CoordinateSpace::World > world_bounding_box {
							world_matrix * primitive.getBoundingBox()
						};

						// No. Skip it
						if ( !intersects( frustum, world_bounding_box ) ) continue;

						//assert( primitive.m_texture );
						const ModelMatrixInfo matrix_info { .model_matrix = world_matrix,
							                                .material_id = primitive.m_material->getID() };

						// If the textureless flag is on and we have a texture then skip the primitive.c
						if ( tree_flags & IS_TEXTURELESS )
						{
							if ( primitive.m_material != nullptr ) continue;
						}
						else
						{
							// Flag is not present
							if ( primitive.m_material == nullptr ) continue;
						}

						const auto key {
							std::make_pair( matrix_info.material_id, primitive.m_index_buffer.getOffset() )
						};

						assert( primitive.m_index_buffer.size() > 0 );

						profiling::addVertexDrawn( primitive.m_index_buffer.size() );

						if ( auto itter = draw_pairs.find( key ); itter != draw_pairs.end() )
						{
							ZoneScopedN( "Accumulate for draw pair" );
							//Draw command for this mesh already exists. Simply add a count to it
							auto& [ itter_key, pair ] = *itter;
							auto& [ existing_cmd, model_matrix ] = pair;

							existing_cmd.instanceCount += 1;
							model_matrix.emplace_back( matrix_info );
							assert( model_matrix.size() == existing_cmd.instanceCount );
						}
						else
						{
							ZoneScopedN( "Create new draw pair" );
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
