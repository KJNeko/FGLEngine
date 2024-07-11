//
// Created by kj16609 on 7/11/24.
//

#include "CameraSwapchain.hpp"

namespace fgl::engine
{

	vk::raii::RenderPass CameraSwapchain::creaeteRenderpass()
	{
		RenderPassBuilder builder;

		builder.registerAttachments( output.composite, output.depth, gbuffer.position, gbuffer.normal, gbuffer.albedo );

		using UsedColorAttachment = UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >;

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthAttachmentOptimal >,
			UsedColorAttachment,
			UsedColorAttachment,
			UsedColorAttachment >
			g_buffer_subpass { 0, output.depth, gbuffer.position, gbuffer.normal, gbuffer.albedo };

		g_buffer_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests );

		g_buffer_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > >
			composite_subpass { 1, output.composite, gbuffer.position, gbuffer.normal, gbuffer.albedo };

		composite_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		// For color attachments
		composite_subpass.registerDependencyFrom(
			g_buffer_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

		composite_subpass.registerDependencyToExternal(
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader );

		//Done
		builder.registerSubpass( g_buffer_subpass );
		builder.registerSubpass( composite_subpass );

		return builder.create();
	}

	std::vector< vk::raii::Framebuffer > CameraSwapchain::createFrambuffers()
	{
		const auto image_count { SwapChain::MAX_FRAMES_IN_FLIGHT };
		gbuffer.position.createResourceSpread( image_count, m_extent );
		gbuffer.normal.createResourceSpread( image_count, m_extent );
		gbuffer.albedo.createResourceSpread( image_count, m_extent );

		output.composite.createResourceSpread( image_count, m_extent, vk::ImageUsageFlagBits::eSampled );
		output.depth.createResourceSpread( image_count, m_extent );

		std::vector< vk::raii::Framebuffer > buffers {};
		buffers.reserve( image_count );

		for ( FrameIndex i = 0; i < image_count; ++i )
		{
			std::vector< vk::ImageView > attachments {
				getViewsForFrame( i, gbuffer.position, gbuffer.normal, gbuffer.albedo, output.composite, output.depth )
			};

			vk::FramebufferCreateInfo info {};
			info.renderPass = m_renderpass;
			info.setAttachments( attachments );
			info.width = m_extent.width;
			info.height = m_extent.height;
			info.layers = 1;

			buffers.emplace_back( Device::getInstance()->createFramebuffer( info ) );
		}

		return buffers;
	}

	CameraSwapchain::CameraSwapchain( const vk::Extent2D extent ) :
	  m_extent( extent ),
	  m_renderpass( creaeteRenderpass() ),
	  m_framebuffers( createFrambuffers() )
	{}

} // namespace fgl::engine