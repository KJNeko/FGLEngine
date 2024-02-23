//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <tracy/TracyC.h>
#include <vulkan/vulkan.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <set>
#include <stdexcept>
#include <thread>

#include "engine/debug/drawers.hpp"
#include "engine/literals/size.hpp"

namespace fgl::engine
{
	std::unique_ptr< Buffer > m_global_draw_parameter_buffer { nullptr };

	void initDrawParameterBuffer( std::uint32_t size )
	{
		m_global_draw_parameter_buffer = std::make_unique< Buffer >(
			size,
			vk::BufferUsageFlagBits::eIndirectBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eDeviceLocal );
	}

	EntityRendererSystem::EntityRendererSystem( Device& device, VkRenderPass render_pass ) : m_device( device )
	{
		ZoneScoped;
		PipelineConfigInfo info { render_pass };

		for ( int i = 0; i < 4; ++i ) PipelineConfigInfo::addColorAttachmentConfig( info );

		info.subpass = 0;

		m_pipeline =
			std::make_unique< Pipeline >( m_device, info, "shaders/gbuffer.vert.spv", "shaders/gbuffer.frag.spv" );

		PipelineConfigInfo composition_info { render_pass };
		PipelineConfigInfo::addColorAttachmentConfig( composition_info );
		PipelineConfigInfo::disableVertexInput( composition_info );
		composition_info.subpass = 1;

		m_composition_pipeline = std::make_unique< CompositionPipeline >(
			m_device, composition_info, "shaders/composition.vert.spv", "shaders/composition.frag.spv" );

		using namespace fgl::literals::size_literals;

		initVertexBuffer( 128_MiB );
		initIndexBuffer( 64_MiB );
		initDrawParameterBuffer( 1_KiB );
	}

	EntityRendererSystem::~EntityRendererSystem()
	{}

	using DrawPair = std::pair< vk::DrawIndexedIndirectCommand, std::vector< ModelMatrixInfo > >;

	bool operator<( const DrawPair& left, const DrawPair& right )
	{
		return left.first.firstIndex < right.first.firstIndex;
	}

	bool operator==( const DrawPair& left, const DrawPair& right )
	{
		return left.first.firstIndex == right.first.firstIndex && left.first.indexCount && right.first.indexCount;
	}

	void EntityRendererSystem::pass( FrameInfo& info )
	{
		ZoneScoped;
		auto& command_buffer { info.command_buffer };
		{
			TracyVkZone( info.tracy_ctx, command_buffer, "Render game objects" );

			//TracyVkZone( m_device.getCurrentTracyCTX(), command_buffer, "Render game objects" );
			m_pipeline->bind( command_buffer );

			m_pipeline->bindDescriptor( command_buffer, 0, info.global_descriptor_set );
			m_pipeline->bindDescriptor( command_buffer, 1, Texture::getTextureDescriptorSet() );

			std::set< DrawPair > draw_pairs;

			for ( auto& [ key, obj ] : info.game_objects )
			{
				TracyVkZone( info.tracy_ctx, command_buffer, "Render game object" );
				if ( obj.model == nullptr ) continue;

				const BoundingBox model_bounding_box {
					obj.model->getBoundingBox( Matrix< MatrixType::ModelToWorld >( obj.transform.mat4() ) )
				};

				debug::world::drawBoundingBox( model_bounding_box, info.camera );

				if ( !model_bounding_box.isInFrustum( info.camera_frustum ) ) continue;

				for ( const auto& primitive : obj.model->m_primitives )
				{
					ZoneScopedN( "Queue Primitive" );
					const ModelMatrixInfo matrix_info { .model_matrix = obj.transform.mat4(),
						                                .texture_idx = primitive.m_texture->getID() };
					//.normal_matrix = obj.transform.normalMatrix() };

					vk::DrawIndexedIndirectCommand cmd;

					cmd.firstIndex = primitive.m_index_buffer.getOffsetCount();
					cmd.indexCount = primitive.m_index_buffer.count();

					cmd.vertexOffset = primitive.m_vertex_buffer.getOffsetCount();

					cmd.instanceCount = 1;

					TracyCZoneN( search_zone_TRACY, "Draw pair deduplicate search", true );
					auto itter = std::find(
						draw_pairs.begin(), draw_pairs.end(), std::make_pair( cmd, std::vector< ModelMatrixInfo >() ) );
					TracyCZoneEnd( search_zone_TRACY );

					if ( itter != draw_pairs.end() )
					{
						ZoneScopedN( "Increment existing render call" );
						//Draw command for this mesh already exists. Simply add a count to it
						auto [ existing_cmd, model_matrix ] = *itter;

						//Sort each model matrix by distance from camera. Render closest first
						const auto camera_pos { info.camera.getPosition() };

						{
							ZoneScopedN( "Sort model matricies by distance" );
							std::sort(
								model_matrix.begin(),
								model_matrix.end(),
								[ camera_pos ]( const ModelMatrixInfo& first, const ModelMatrixInfo& second ) -> bool
								{
									const auto& first_pos_v4 { first.model_matrix[ 3 ] };
									const auto& second_pos_v4 { second.model_matrix[ 3 ] };

									const glm::vec3 first_pos { first_pos_v4.x, first_pos_v4.y, first_pos_v4.z };
									const glm::vec3 second_pos { second_pos_v4.x, second_pos_v4.y, second_pos_v4.z };

									const auto first_distance { glm::distance( first_pos, camera_pos ) };
									const auto second_distance { glm::distance( second_pos, camera_pos ) };

									return first_distance < second_distance;
								} );
						}

						draw_pairs.erase( itter );
						existing_cmd.instanceCount++;
						model_matrix.emplace_back( matrix_info );
						draw_pairs.emplace( existing_cmd, std::move( model_matrix ) );
					}
					else
					{
						ZoneScopedN( "Create new render call" );
						draw_pairs.emplace( cmd, std::vector< ModelMatrixInfo > { matrix_info } );
					}
				}
			}

			if ( draw_pairs.empty() )
			{
				std::cout << "Nothing to draw!" << std::endl;
				command_buffer.nextSubpass( vk::SubpassContents::eInline );
				return;
			}

			std::vector< vk::DrawIndexedIndirectCommand > draw_commands;
			std::vector< ModelMatrixInfo > model_matrices;

			TracyCZoneN( filter_zone_TRACY, "Reorganize draw commands", true );
			for ( auto& itter : draw_pairs )
			{
				auto cmd { itter.first };
				cmd.firstInstance = model_matrices.size();
				auto& matricies { itter.second };

				draw_commands.emplace_back( cmd );
				model_matrices.insert( model_matrices.end(), matricies.begin(), matricies.end() );
			}
			TracyCZoneEnd( filter_zone_TRACY );

			TracyCZoneN( draw_zone_TRACY, "Submit draw data", true );
			auto& draw_parameter_buffer { m_draw_parameter_buffers[ info.frame_idx ] };

			if ( draw_parameter_buffer == nullptr || draw_parameter_buffer->count() != draw_commands.size() )
			{
				draw_parameter_buffer =
					std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );
			}
			else
			{
				//Simply set and flush
				*draw_parameter_buffer = draw_commands;
			}
			TracyCZoneEnd( draw_zone_TRACY );

			draw_parameter_buffer->flush();

			auto& model_matrix_info_buffer { m_model_matrix_info_buffers[ info.frame_idx ] };

			model_matrix_info_buffer =
				std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matrices );

			model_matrix_info_buffer->flush();

			auto& model_matricies_suballoc { model_matrix_info_buffer };
			auto& draw_params { draw_parameter_buffer };

			std::vector< vk::Buffer > vertex_buffers { m_vertex_buffer->getVkBuffer(),
				                                       model_matricies_suballoc->getVkBuffer() };

			command_buffer.bindVertexBuffers( 0, vertex_buffers, { 0, model_matricies_suballoc->getOffset() } );
			command_buffer.bindIndexBuffer( m_index_buffer->getVkBuffer(), 0, vk::IndexType::eUint32 );

			command_buffer.drawIndexedIndirect(
				draw_params->getVkBuffer(), draw_params->getOffset(), draw_params->count(), draw_params->stride() );

			command_buffer.nextSubpass( vk::SubpassContents::eInline );
		}

		{
			//Composition pass
			TracyVkZone( info.tracy_ctx, command_buffer, "Composition pass" );
			m_composition_pipeline->bind( command_buffer );

			m_composition_pipeline->bindDescriptor( command_buffer, 0, info.gbuffer_descriptor_set );

			command_buffer.draw( 3, 1, 0, 0 );
		}
	}

} // namespace fgl::engine
