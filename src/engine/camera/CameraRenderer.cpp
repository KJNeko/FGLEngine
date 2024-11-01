//
// Created by kj16609 on 7/21/24.
//

#include "CameraRenderer.hpp"

#include "CameraSwapchain.hpp"
#include "engine/rendering/renderpass/RenderPass.hpp"

namespace fgl::engine
{
	class CameraSwapchain;

	vk::raii::RenderPass CameraRenderer::createRenderPass()
	{
		rendering::RenderPassBuilder builder {};

		builder.setAttachmentCount( 7 );

		// Set formats for each item in the swapchain

		//XYZ in world space
		auto position { builder.attachment( POSITION_INDEX ) };
		position.setFormat( vk::Format::eR16G16B16A16Sfloat ); // position
		position.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal );
		position.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		//RGBA
		auto normal { builder.attachment( NORMAL_INDEX ) };
		normal.setFormat( vk::Format::eR16G16B16A16Sfloat ); // normal
		normal.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal );
		normal.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		// RGBA
		auto color { builder.attachment( COLOR_INDEX ) };
		color.setFormat( vk::Format::eR8G8B8A8Unorm ); // color
		color.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal );
		color.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		// Metallic, Roughness, Occlusion
		auto metallic_roughness { builder.attachment( METALLIC_INDEX ) };
		metallic_roughness.setFormat( vk::Format::eR16G16B16A16Sfloat );
		metallic_roughness.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal );
		metallic_roughness.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		// RGB
		auto emissive { builder.attachment( EMISSIVE_INDEX ) };
		emissive.setFormat( vk::Format::eR16G16B16A16Sfloat );
		emissive.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal );
		emissive.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		auto composite { builder.attachment( COMPOSITE_INDEX ) };
		//TODO: For HDR I think this needs to be a bigger range then 8bits per channel.
		composite.setFormat( vk::Format::eR8G8B8A8Unorm ); // composite
		composite.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal );
		composite.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		auto depth { builder.attachment( DEPTH_INDEX ) };
		depth.setLayouts( vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilReadOnlyOptimal );
		depth.setFormat( SwapChain::findDepthFormat() ); // depth
		depth.setOps( vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore );

		auto& g_buffer_subpass { builder.createSubpass( 0 ) };
		g_buffer_subpass.setDepthLayout( DEPTH_INDEX, vk::ImageLayout::eDepthStencilAttachmentOptimal );
		g_buffer_subpass.addRenderLayout( COLOR_INDEX, vk::ImageLayout::eColorAttachmentOptimal );
		g_buffer_subpass.addRenderLayout( POSITION_INDEX, vk::ImageLayout::eColorAttachmentOptimal );
		g_buffer_subpass.addRenderLayout( NORMAL_INDEX, vk::ImageLayout::eColorAttachmentOptimal );
		g_buffer_subpass.addRenderLayout( METALLIC_INDEX, vk::ImageLayout::eColorAttachmentOptimal );
		g_buffer_subpass.addRenderLayout( EMISSIVE_INDEX, vk::ImageLayout::eColorAttachmentOptimal );

		g_buffer_subpass.addDependencyFromExternal(
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests );

		g_buffer_subpass.addDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		auto& composite_subpass { builder.createSubpass( 1 ) };
		composite_subpass.addRenderLayout( COMPOSITE_INDEX, vk::ImageLayout::eColorAttachmentOptimal );
		composite_subpass.addInputLayout( COLOR_INDEX, vk::ImageLayout::eShaderReadOnlyOptimal );
		composite_subpass.addInputLayout( POSITION_INDEX, vk::ImageLayout::eShaderReadOnlyOptimal );
		composite_subpass.addInputLayout( NORMAL_INDEX, vk::ImageLayout::eShaderReadOnlyOptimal );
		composite_subpass.addInputLayout( METALLIC_INDEX, vk::ImageLayout::eShaderReadOnlyOptimal );
		composite_subpass.addInputLayout( EMISSIVE_INDEX, vk::ImageLayout::eShaderReadOnlyOptimal );

		composite_subpass.addDependency(
			g_buffer_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader );

		composite_subpass.addDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		return builder.create();
	}

	void CameraRenderer::setViewport( const vk::raii::CommandBuffer& command_buffer, const vk::Extent2D extent )
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

	void CameraRenderer::setScissor( const vk::raii::CommandBuffer& command_buffer, const vk::Extent2D extent )
	{
		const vk::Rect2D scissor { { 0, 0 }, extent };

		const std::vector< vk::Rect2D > scissors { scissor };

		command_buffer.setScissor( 0, scissors );
	}

	void CameraRenderer::beginRenderPass(
		const vk::raii::CommandBuffer& command_buffer, CameraSwapchain& swapchain, const FrameIndex index )
	{
		vk::RenderPassBeginInfo info {};
		info.renderPass = m_renderpass;
		info.framebuffer = swapchain.getFramebuffer( index );
		info.renderArea = { .offset = { 0, 0 }, .extent = swapchain.getExtent() };

		info.setClearValues( swapchain.getClearValues() );

		command_buffer.beginRenderPass( info, vk::SubpassContents::eInline );

		setViewport( command_buffer, swapchain.getExtent() );
		setScissor( command_buffer, swapchain.getExtent() );
	}

	void CameraRenderer::endRenderPass( const vk::raii::CommandBuffer& command_buffer )
	{
		command_buffer.endRenderPass();
	}

	void CameraRenderer::pass( FrameInfo& frame_info, CameraSwapchain& camera_swapchain )
	{
		ZoneScopedN( "CameraRenderer::pass" );
		m_culling_system.startPass( frame_info );

		auto& command_buffer { frame_info.command_buffer };

		beginRenderPass( command_buffer, camera_swapchain, frame_info.frame_idx );

		m_culling_system.wait();

		//m_terrain_system.pass( frame_info );

		m_entity_renderer.pass( frame_info );
		m_line_drawer.pass( frame_info );

		m_composition_system.pass( frame_info );

		endRenderPass( command_buffer );
	}

	vk::raii::RenderPass& CameraRenderer::getRenderpass()
	{
		return m_renderpass;
	}

} // namespace fgl::engine
