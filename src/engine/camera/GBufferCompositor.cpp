//
// Created by kj16609 on 1/13/25.
//
#include "GBufferCompositor.hpp"

#include "Camera.hpp"
#include "CompositeSwapchain.hpp"
#include "GBufferSwapchain.hpp"
#include "engine/rendering/pipelines/v2/AttachmentBuilder.hpp"

namespace fgl::engine
{

	void GBufferCompositor::setViewport( const vk::raii::CommandBuffer& command_buffer, const vk::Extent2D extent )
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast< float >( extent.width );
		viewport.height = static_cast< float >( extent.height );

		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const std::vector< vk::Viewport > viewports { viewport };

		command_buffer.setViewport( 0, viewports );
	}

	void GBufferCompositor::setScissor( const vk::raii::CommandBuffer& command_buffer, const vk::Extent2D extent )
	{
		const vk::Rect2D scissor { { 0, 0 }, extent };

		const std::vector< vk::Rect2D > scissors { scissor };

		command_buffer.setScissor( 0, scissors );
	}

	void GBufferCompositor::
		beginPass( vk::raii::CommandBuffer& cmd, CompositeSwapchain& swapchain, const FrameIndex& index )
	{
		const vk::RenderingInfo info { swapchain.getRenderingInfo( index ) };

		cmd.beginRendering( info );

		setViewport( cmd, swapchain.getExtent() );
		setScissor( cmd, swapchain.getExtent() );
	}

	void GBufferCompositor::endPass( vk::raii::CommandBuffer& cmd )
	{
		cmd.endRendering();
	}

	GBufferCompositor::GBufferCompositor( const CompositeFlags flags ) : m_flags( flags )
	{
		PipelineBuilder builder { 0 };

		builder.addDescriptorSet( gbuffer_set );
		builder.addDescriptorSet( Camera::getDescriptorLayout() );

		builder.addColorAttachment().setFormat( pickColorFormat() ).finish();

		builder.setPushConstant( vk::ShaderStageFlagBits::eFragment, sizeof( CompositionControl ) );

		builder.setVertexShader( Shader::loadVertex( "shaders/composition.slang" ) );
		builder.setFragmentShader( Shader::loadFragment( "shaders/composition.slang" ) );

		builder.disableCulling();
		builder.disableVertexInput();

		m_pipeline = builder.create();

		m_pipeline->setDebugName( "Composition pipeline" );
	}

	void GBufferCompositor::
		composite( vk::raii::CommandBuffer& command_buffer, Camera& camera, const FrameIndex frame_index )
	{
		auto& gbuffer_swapchain { camera.getSwapchain() };
		auto& composite_swapchain { camera.getCompositeSwapchain() };

		composite_swapchain.transitionImages( command_buffer, CompositeSwapchain::INITAL, frame_index );

		beginPass( command_buffer, composite_swapchain, frame_index );

		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, gbuffer_swapchain.getGBufferDescriptor( frame_index ) );
		m_pipeline->bindDescriptor( command_buffer, camera.getDescriptor( frame_index ) );

		m_pipeline->pushConstant( command_buffer, vk::ShaderStageFlagBits::eFragment, m_control );

		command_buffer.draw( 3, 1, 0, 0 );

		endPass( command_buffer );

		composite_swapchain.transitionImages( command_buffer, CompositeSwapchain::FINAL, frame_index );
	}

} // namespace fgl::engine
