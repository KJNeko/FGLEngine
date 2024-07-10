//
// Created by kj16609 on 7/10/24.
//

#include "CameraSwapchain.hpp"

#include "Device.hpp"
#include "RenderPassBuilder.hpp"

/*
namespace fgl::engine
{

	vk::Format pickOptimalDepthFormat()
	{
		return Device::getInstance().findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment );
	}

	std::vector< vk::raii::Framebuffer > CameraSwapchain::createFramebuffers()
	{}

	vk::raii::RenderPass CameraSwapchain::createRenderpass()
	{
		//In order to make the renderpass we need to know what we are doing.
		RenderPassBuilder render_pass_builder;

		//We start by defining the various attachments used here.

		auto& [ color_attachment, depth_attachment, position_attachment, normal_attachment, albedo_attachment ] =
			m_attachments;

		render_pass_builder.registerAttachments(
			color_attachment, depth_attachment, position_attachment, normal_attachment, albedo_attachment );

		//TODO: Pre depth pass in order to reduce the number of fragment invocations

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal > >
			g_buffer_subpass { 0, depth_attachment, position_attachment, normal_attachment, albedo_attachment };

		g_buffer_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests );

		g_buffer_subpass.registerDependencyFromExternal(
			vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput );

		Subpass<
			vk::PipelineBindPoint::eGraphics,
			UsedAttachment< DepthAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal >,
			UsedAttachment< ColorAttachment, vk::ImageLayout::eColorAttachmentOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal >,
			InputAttachment< ColorAttachment, vk::ImageLayout::eShaderReadOnlyOptimal > >
			composite_subpass {
				1, depth_attachment, color_attachment, position_attachment, normal_attachment, albedo_attachment
			};

		// For color attachments
		composite_subpass.registerDependencyFrom(
			g_buffer_subpass,
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

		// For depth attachment
		composite_subpass.registerDependencyFrom(
			g_buffer_subpass,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::DependencyFlagBits::eByRegion );

		composite_subpass.registerDependencyToExternal(
			vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eInputAttachmentRead,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlagBits::eByRegion );

		render_pass_builder.registerSubpass( g_buffer_subpass );
		render_pass_builder.registerSubpass( composite_subpass );

		return render_pass_builder.create();
	}

	CameraSwapchain::CameraSwapchain( PhysicalDevice& physical_device, vk::Extent2D starting_extent ) :
	  m_physical_device( physical_device ),
	  m_final_render_info( vk::Format::eR8G8B8A8Unorm, pickOptimalDepthFormat() ),
	  m_attachments( m_final_render_info.m_color_format, m_final_render_info.m_depth_format ),
	  m_extent( starting_extent ),
	  m_renderpass( createRenderpass() ),
	  m_framebuffers( createFramebuffers() )
	{}

} // namespace fgl::engine
*/