//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#include <tracy/TracyC.h>
#include <vulkan/vulkan.hpp>

#include <set>

#include "DrawPair.hpp"
#include "engine/literals/size.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"
#include "spdlog/fmt/bundled/compile.h"

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

	EntityRendererSystem::EntityRendererSystem( Device& device, vk::raii::RenderPass& render_pass ) : m_device( device )
	{
		ZoneScoped;
		PipelineConfigInfo standard_info { render_pass };
		for ( int i = 0; i < 4; ++i ) PipelineConfigInfo::addColorAttachmentConfig( standard_info );
		standard_info.subpass = 0;
		m_standard_pipeline = std::make_unique< StandardPipeline >( m_device, std::move( standard_info ) );
		m_standard_pipeline->setDebugName( "Standard entity pipeline" );

		PipelineConfigInfo textured_info { render_pass };
		for ( int i = 0; i < 4; ++i ) PipelineConfigInfo::addColorAttachmentConfig( textured_info );
		textured_info.subpass = 0;
		m_textured_pipeline = std::make_unique< TexturedPipeline >( m_device, std::move( textured_info ) );
		m_textured_pipeline->setDebugName( "Textured entity pipeline" );

		using namespace fgl::literals::size_literals;

		initVertexBuffer( 512_MiB );
		initIndexBuffer( 128_MiB );
		initDrawParameterBuffer( 1_KiB );
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
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Render entities" );

		texturelessPass( info );
		//texturedPass( info );

		/*
		auto [ draw_commands, model_matricies ] =
			getDrawCallsFromTree( info.game_objects, info.camera_frustum, IS_VISIBLE | IS_ENTITY );

		if ( draw_commands.size() == 0 ) return;

		//Setup model matrix info buffers
		auto& model_matrix_info_buffer { m_model_matrix_info_buffers[ info.frame_idx ] };

		model_matrix_info_buffer =
			std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matricies );

		assert( model_matrix_info_buffer->size() == model_matricies.size() );

		// Setup draw parameter buffer
		TracyCZoneN( draw_zone_TRACY, "Submit draw data", true );
		auto& draw_parameter_buffer { m_draw_parameter_buffers[ info.frame_idx ] };

		draw_parameter_buffer =
			std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );

		const auto& draw_params { draw_parameter_buffer };
		assert( draw_params->size() == draw_commands.size() );
		assert( draw_params->stride() == sizeof( vk::DrawIndexedIndirectCommand ) );

		TracyCZoneEnd( draw_zone_TRACY );

		const std::vector< vk::Buffer > vertex_buffers { m_vertex_buffer->getVkBuffer(),
			                                             model_matrix_info_buffer->getVkBuffer() };

		command_buffer.bindVertexBuffers( 0, vertex_buffers, { 0, model_matrix_info_buffer->getOffset() } );
		command_buffer.bindIndexBuffer( m_index_buffer->getVkBuffer(), 0, vk::IndexType::eUint32 );

#if ENABLE_IMGUI
		ImGui::Text( "Indirect draws: %lu", static_cast< std::size_t >( draw_params->size() ) );
#endif

		command_buffer.drawIndexedIndirect(
			draw_params->getVkBuffer(), draw_params->getOffset(), draw_params->size(), draw_params->stride() );
		*/
	}

	void EntityRendererSystem::texturelessPass( FrameInfo& info )
	{
		ZoneScopedN( "Textureless pass" );
		auto& command_buffer { info.command_buffer };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Render textureless entities" );

		//Bind pipeline
		m_standard_pipeline->bind( command_buffer );

		//Bind only descriptor we need.
		m_standard_pipeline
			->bindDescriptor( command_buffer, GlobalDescriptorSet::m_set_idx, info.global_descriptor_set );

		//Get all commands for drawing anything without a texture
		auto [ draw_commands, model_matricies ] =
			getDrawCallsFromTree( info.game_objects, info.camera_frustum, IS_VISIBLE | IS_ENTITY, TEXTURELESS );

		//TODO: Filter Textureless models (#6)

		if ( draw_commands.size() == 0 ) return;

		auto& model_matrix_info_buffer { m_simple_model_matrix_info_buffers[ info.frame_idx ] };

		model_matrix_info_buffer =
			std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matricies );

		auto& draw_parameter_buffer { m_draw_simple_parameter_buffers[ info.frame_idx ] };

		draw_parameter_buffer =
			std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );

		const std::vector< vk::Buffer > vert_buffers { m_vertex_buffer->getVkBuffer(),
			                                           model_matrix_info_buffer->getVkBuffer() };

		command_buffer.bindVertexBuffers( 0, vert_buffers, { 0, model_matrix_info_buffer->getOffset() } );
		command_buffer.bindIndexBuffer( m_index_buffer->getVkBuffer(), 0, vk::IndexType::eUint32 );

		command_buffer.drawIndexedIndirect(
			draw_parameter_buffer->getVkBuffer(),
			draw_parameter_buffer->getOffset(),
			draw_parameter_buffer->size(),
			draw_parameter_buffer->stride() );
	}

} // namespace fgl::engine
