//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ON
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

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

	void EntityRendererSystem::pass( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };
		{
			TracyVkZone( info.tracy_ctx, command_buffer, "Render game objects" );

			//TracyVkZone( m_device.getCurrentTracyCTX(), command_buffer, "Render game objects" );
			m_pipeline->bind( command_buffer );

			m_pipeline->bindDescriptor( command_buffer, 0, info.global_descriptor_set );
			m_pipeline->bindDescriptor( command_buffer, 1, Texture::getTextureDescriptorSet() );

			std::vector< vk::DrawIndexedIndirectCommand > draw_commands;
			std::vector< ModelMatrixInfo > model_matrices;

			for ( auto& [ key, obj ] : info.game_objects )
			{
				TracyVkZone( info.tracy_ctx, command_buffer, "Render game object" );
				if ( obj.model == nullptr ) continue;

				for ( const auto& primitive : obj.model->m_primitives )
				{
					const ModelMatrixInfo matrix_info { .model_matrix = obj.transform.mat4(),
						                                .texture_idx = primitive.m_texture->getID() };
					//.normal_matrix = obj.transform.normalMatrix() };

					vk::DrawIndexedIndirectCommand cmd;

					cmd.firstIndex = primitive.m_index_buffer.getOffsetCount();
					cmd.indexCount = primitive.m_index_buffer.count();

					cmd.vertexOffset = primitive.m_vertex_buffer.getOffsetCount();

					cmd.firstInstance = model_matrices.size();
					cmd.instanceCount = 1;

					draw_commands.emplace_back( cmd );
					model_matrices.emplace_back( matrix_info );
				}
			}

			assert( draw_commands.size() > 0 && "No draw commands to render" );

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
