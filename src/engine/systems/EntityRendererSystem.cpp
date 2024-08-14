//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#include <tracy/TracyC.h>
#include <vulkan/vulkan.hpp>

#include "DrawPair.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/profiling/counters.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine
{
	EntityRendererSystem::EntityRendererSystem( Device& device, vk::raii::RenderPass& render_pass ) : m_device( device )
	{
		ZoneScoped;
		{
			PipelineConfigInfo standard_info { render_pass };
			PipelineConfigInfo::addGBufferAttachmentsConfig( standard_info );

			standard_info.subpass = 0;
			m_standard_pipeline = std::make_unique< StandardPipeline >( m_device, std::move( standard_info ) );
			m_standard_pipeline->setDebugName( "Standard entity pipeline" );
		}

		{
			PipelineConfigInfo textured_info { render_pass };
			PipelineConfigInfo::addGBufferAttachmentsConfig( textured_info );

			textured_info.subpass = 0;
			m_textured_pipeline = std::make_unique< TexturedPipeline >( m_device, std::move( textured_info ) );
			m_textured_pipeline->setDebugName( "Textured entity pipeline" );
		}
	}

	vk::raii::CommandBuffer& EntityRendererSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };

		//This function becomes a dummy since we have multiple pipelines.
		//We will instead bind the pipeline and descriptors for each stage of this pass.

		//m_pipeline->bind( command_buffer );

		//m_pipeline->bindDescriptor( command_buffer, 0, info.global_descriptor_set );
		//m_pipeline->bindDescriptor( command_buffer, 1, Texture::getTextureDescriptorSet() );

		return command_buffer;
	}

	void EntityRendererSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "Entity pass" );
		[[maybe_unused]] auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Render entities" );

		texturelessPass( info );
		texturedPass( info );
	}

	void EntityRendererSystem::texturelessPass( FrameInfo& info )
	{
		ZoneScopedN( "Textureless pass" );
		auto& command_buffer { info.command_buffer };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Render textureless entities" );

		//Bind pipeline
		m_standard_pipeline->bind( command_buffer );

		m_standard_pipeline
			->bindDescriptor( command_buffer, CameraDescriptorSet::m_set_idx, info.getCameraDescriptor() );

		//Get all commands for drawing anything without a texture
		auto [ draw_commands, model_matricies ] = getDrawCallsFromTree(
			info.game_objects, info.camera->getFrustumBounds(), IS_VISIBLE | IS_ENTITY, IS_TEXTURELESS );

		//TODO: Filter Textureless models (#6)

		if ( draw_commands.size() == 0 ) return;

		auto& model_matrix_info_buffer { m_simple_model_matrix_info_buffers[ info.frame_idx ] };

		model_matrix_info_buffer =
			std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matricies );

		auto& draw_parameter_buffer { m_draw_simple_parameter_buffers[ info.frame_idx ] };

		draw_parameter_buffer =
			std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );

		const std::vector< vk::Buffer > vert_buffers { info.model_vertex_buffer.getVkBuffer(),
			                                           model_matrix_info_buffer->getVkBuffer() };

		command_buffer.bindVertexBuffers( 0, vert_buffers, { 0, model_matrix_info_buffer->getOffset() } );
		command_buffer.bindIndexBuffer( info.model_index_buffer.getVkBuffer(), 0, vk::IndexType::eUint32 );

		command_buffer.drawIndexedIndirect(
			draw_parameter_buffer->getVkBuffer(),
			draw_parameter_buffer->getOffset(),
			draw_parameter_buffer->size(),
			draw_parameter_buffer->stride() );
	}

	void EntityRendererSystem::texturedPass( FrameInfo& info )
	{
		ZoneScopedN( "Textured pass" );
		auto& command_buffer { info.command_buffer };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Render textured entities" );

		m_textured_pipeline->bind( command_buffer );

		// Since the camera was bound in the textureless pass we shouldn't need to bind it here too.
		// m_textured_pipeline
		// ->bindDescriptor( command_buffer, CameraDescriptorSet::m_set_idx, info.global_descriptor_set );

		m_textured_pipeline
			->bindDescriptor( command_buffer, TextureDescriptorSet::m_set_idx, Texture::getTextureDescriptorSet() );

		auto [ draw_commands, model_matricies ] =
			getDrawCallsFromTree( info.game_objects, info.camera->getFrustumBounds(), IS_VISIBLE | IS_ENTITY );

		if ( draw_commands.empty() ) return;

		profiling::addModelDrawn( model_matricies.size() );

		auto& model_matrix_info_buffer { m_textured_model_matrix_info_buffers[ info.frame_idx ] };
		model_matrix_info_buffer =
			std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matricies );

		auto& draw_parameter_buffer { m_draw_textured_parameter_buffers[ info.frame_idx ] };
		draw_parameter_buffer =
			std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );

		const std::vector< vk::Buffer > vert_buffers { info.model_vertex_buffer.getVkBuffer(),
			                                           model_matrix_info_buffer->getVkBuffer() };

		command_buffer.bindVertexBuffers( 0, vert_buffers, { 0, model_matrix_info_buffer->getOffset() } );
		command_buffer.bindIndexBuffer( info.model_index_buffer.getVkBuffer(), 0, vk::IndexType::eUint32 );

		command_buffer.drawIndexedIndirect(
			draw_parameter_buffer->getVkBuffer(),
			draw_parameter_buffer->getOffset(),
			draw_parameter_buffer->size(),
			draw_parameter_buffer->stride() );
	};

} // namespace fgl::engine
