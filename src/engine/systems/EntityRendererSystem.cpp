//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#include <imgui/imgui.h>
#include <tracy/TracyC.h>
#include <vulkan/vulkan.hpp>

#include <set>

#include "DrawPair.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/literals/size.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

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

		m_pipeline = std::make_unique< Pipeline >( m_device, info );
		m_pipeline->setDebugName( "Entity pipeline" );

		using namespace fgl::literals::size_literals;

		initVertexBuffer( 128_MiB );
		initIndexBuffer( 64_MiB );
		initDrawParameterBuffer( 1_KiB );
	}

	vk::CommandBuffer& EntityRendererSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };
		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, 0, info.global_descriptor_set );
		m_pipeline->bindDescriptor( command_buffer, 1, Texture::getTextureDescriptorSet() );

		return command_buffer;
	}

	void EntityRendererSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "Entity pass" );
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, command_buffer, "Render entities" );

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
	}

} // namespace fgl::engine
