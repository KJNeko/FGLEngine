//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#include <tracy/TracyC.h>
#include <vulkan/vulkan.hpp>

#include "EngineContext.hpp"
#include "assets/model/ModelVertex.hpp"
#include "engine/assets/material/Material.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/debug/timing/FlameGraph.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/pipelines/v2/PipelineBuilder.hpp"
#include "memory/buffers/BufferHandle.hpp"

namespace fgl::engine
{
	EntityRendererSystem::EntityRendererSystem()
	{
		ZoneScoped;

		// PipelineConfigInfo textured_info { render_pass };
		// PipelineConfigInfo::addGBufferAttachmentsConfig( textured_info );

		PipelineBuilder builder { 0 };

		addGBufferAttachments( builder );

		builder.addDescriptorSet( Camera::getDescriptorLayout() );
		builder.addDescriptorSet( Texture::getDescriptorLayout() );
		builder.addDescriptorSet( Material::getDescriptorLayout() );

		builder.setFragmentShader( Shader::loadFragment( "shaders/textured.slang" ) );
		builder.setVertexShader( Shader::loadVertex( "shaders/textured.slang" ) );

		builder.setAttributeDescriptions( ModelVertex::getAttributeDescriptions() );
		builder.setBindingDescriptions( ModelVertex::getBindingDescriptions() );

		builder.addDepthAttachment();

		m_textured_pipeline = builder.create();
		m_textured_pipeline->setDebugName( "Textured entity pipeline" );
	}

	EntityRendererSystem::~EntityRendererSystem()
	{}

	CommandBuffer& EntityRendererSystem::setupSystem( const FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer.render_cb };

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
		TracyVkZone( info.tracy_ctx, **command_buffer, "Render entities" );
		auto timer = debug::timing::push( "Render entities" );

		// texturelessPass( info );
		texturedPass( info );
	}

	void EntityRendererSystem::texturedPass( const FrameInfo& info )
	{
		ZoneScopedN( "Textured pass" );
		auto& command_buffer { info.command_buffer.render_cb };
		TracyVkZone( info.tracy_ctx, **command_buffer, "Render textured entities" );

		// compute shader

		if ( info.game_objects.empty() ) return;

		m_textured_pipeline->bind( command_buffer );

		m_textured_pipeline->bindDescriptor( command_buffer, info.getCameraDescriptor() );
		m_textured_pipeline->bindDescriptor( command_buffer, Texture::getDescriptorSet() );
		m_textured_pipeline->bindDescriptor( command_buffer, Material::getDescriptorSet() );

		auto& model_buffers { getModelBuffers() };

		const std::vector< vk::Buffer > vert_buffers {
			model_buffers.m_vertex_buffer->getVkBuffer(),
			model_buffers.m_generated_instance_info[ info.in_flight_idx ].getVkBuffer()
		};

		command_buffer->bindVertexBuffers(
			0, vert_buffers, { 0, model_buffers.m_generated_instance_info[ info.in_flight_idx ].getOffset() } );
		command_buffer->bindIndexBuffer( model_buffers.m_index_buffer->getVkBuffer(), 0, vk::IndexType::eUint32 );

		command_buffer->drawIndexedIndirect(
			info.m_commands.getVkBuffer(),
			info.m_commands.getOffset(),
			info.m_commands.size(),
			info.m_commands.stride() );
	};

} // namespace fgl::engine
