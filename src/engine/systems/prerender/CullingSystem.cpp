//
// Created by kj16609 on 2/23/24.
//

#include "CullingSystem.hpp"

#include <tracy/TracyC.h>

#include "assets/model/Model.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/camera/Camera.hpp"

namespace fgl::engine
{
	static bool enable_culling { true };

	[[maybe_unused]] static bool& isCullingEnabled()
	{
		return enable_culling;
	}

	struct CullPushConstants
	{
		std::uint32_t draw_count;
		std::uint32_t start_idx;
	};

	CullingSystem::CullingSystem()
	{
		PipelineBuilder builder { 0 };

		builder.addDescriptorSet( PRIMITIVE_SET );
		builder.addDescriptorSet( INSTANCES_SET );
		builder.addDescriptorSet( COMMANDS_SET );

		builder.addPushConstants< CullPushConstants >( vk::ShaderStageFlagBits::eCompute );

		builder.setComputeShader( Shader::loadCompute( "shaders/culling.slang" ) );

		m_cull_compute = builder.create();
		m_cull_compute->setDebugName( "Culling" );
	}

	CullingSystem::~CullingSystem()
	{}

	void CullingSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "Culling pass" );

		const auto frustum { info.camera->getFrustumBounds() };

		if ( !enable_culling )
		{
			return;
		}

		auto& command_buffer { info.command_buffer.render_cb };

		m_cull_compute->bind( command_buffer );

		m_cull_compute->bindDescriptor( command_buffer, info.m_primitives_desc ); // primitive set
		m_cull_compute->bindDescriptor( command_buffer, info.m_instances_desc ); // instances
		m_cull_compute->bindDescriptor( command_buffer, *info.m_command_buffer_desc ); // commands output

		CullPushConstants push_constants {};
		push_constants.draw_count = info.m_commands.size();

		command_buffer->pushConstants<
			CullPushConstants >( m_cull_compute->layout(), vk::ShaderStageFlagBits::eCompute, 0, { push_constants } );

		constexpr std::uint32_t dispatch_count { 64 };

		// calculate number of workgroups needed
		const std::uint32_t group_count { ( push_constants.draw_count + dispatch_count - 1 ) / dispatch_count };

		command_buffer->dispatch( group_count, 1, 1 );

		// Add a memory barrier to ensure synchronization between the compute and subsequent stages
		vk::MemoryBarrier memory_barrier {
			vk::AccessFlagBits::eShaderWrite,
			vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eIndirectCommandRead,
		};

		command_buffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader, // Source: Compute shader
			vk::PipelineStageFlagBits::eDrawIndirect | // Destination: Indirect drawing
				vk::PipelineStageFlagBits::eVertexInput
				| vk::PipelineStageFlagBits::eFragmentShader, // ... and vertex input
			{}, // Dependency flags
			{ memory_barrier }, // Memory barrier for synchronization
			{}, // No buffer barrier
			{} // No image barrier
		);
	}

} // namespace fgl::engine
