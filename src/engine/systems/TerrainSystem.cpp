//
// Created by kj16609 on 3/11/24.
//

#include "TerrainSystem.hpp"

#include <tracy/Tracy.hpp>

#include "DrawPair.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/literals/size.hpp"

namespace fgl::engine
{

	/*
	TerrainSystem::TerrainSystem( Device& device, vk::raii::RenderPass& render_pass )
	{
		ZoneScoped;
		PipelineConfigInfo info { render_pass };

		PipelineConfigInfo::setPointPatch( info );
		PipelineConfigInfo::setQuadTesselation( info );

		info.assembly_info.topology = vk::PrimitiveTopology::ePatchList;

		for ( int i = 0; i < 3; ++i ) PipelineConfigInfo::addColorAttachmentConfig( info );

		info.subpass = 0;

		m_pipeline = std::make_unique< Pipeline >( device, std::move( info ) );
		m_pipeline->setDebugName( "Terrain pipeline" );

		using namespace fgl::literals::size_literals;

		initVertexBuffer( 16_MiB );
		initIndexBuffer( 2_MiB );

		this->m_index_buffer->setDebugName( "Terrain index buffer" );
		this->m_vertex_buffer->setDebugName( "Terrain vertex buffer" );
	}

	vk::raii::CommandBuffer& TerrainSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };
		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, 0, info.global_descriptor_set );
		m_pipeline->bindDescriptor( command_buffer, 1, Texture::getTextureDescriptorSet() );

		return info.command_buffer;
	}

	void TerrainSystem::pass( [[maybe_unused]] FrameInfo& info )
	{
		ZoneScopedN( "Terrain pass" );
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Render terrain" );

		return;

		auto [ draw_commands, model_matricies ] =
			getDrawCallsFromTree( info.game_objects, info.camera_data.camera.getFrustumBounds(), IS_VISIBLE );

		if ( draw_commands.size() == 0 ) return;

		//Load commands and matricies into buffer
		auto& draw_parameter_buffer { m_draw_parameter_buffers[ info.frame_idx ] };
		draw_parameter_buffer =
			std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );

		auto& model_matrix_info_buffer { m_model_matrix_info_buffers[ info.frame_idx ] };

		model_matrix_info_buffer =
			std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matricies );

		const std::vector< vk::Buffer > vertex_buffers { m_vertex_buffer->getVkBuffer(),
			                                             model_matrix_info_buffer->getVkBuffer() };

		command_buffer.bindVertexBuffers( 0, vertex_buffers, { 0, model_matrix_info_buffer->getOffset() } );
		command_buffer.bindIndexBuffer( m_index_buffer->getVkBuffer(), 0, vk::IndexType::eUint32 );

		command_buffer.drawIndexedIndirect(
			draw_parameter_buffer->getVkBuffer(),
			draw_parameter_buffer->getOffset(),
			draw_parameter_buffer->size(),
			draw_parameter_buffer->stride() );
	}
	*/

} // namespace fgl::engine