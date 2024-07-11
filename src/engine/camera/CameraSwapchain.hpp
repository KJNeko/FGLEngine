//
// Created by kj16609 on 7/11/24.
//

#pragma once

#include "engine/rendering/Attachment.hpp"
#include "engine/rendering/SwapChain.hpp"

namespace fgl::engine
{

	class CameraSwapchain
	{
		struct
		{
			ColorAttachment position { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment normal { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment albedo { vk::Format::eR8G8B8A8Unorm };
		} gbuffer {};

		struct
		{
			//Final attachments
			ColorAttachment composite { vk::Format::eR8G8B8A8Unorm };
			DepthAttachment depth { SwapChain::findDepthFormat() };
		} output {};

		vk::Extent2D m_extent;

		vk::raii::RenderPass m_renderpass;
		std::vector< vk::raii::Framebuffer > m_framebuffers;

	  public:

		vk::raii::RenderPass creaeteRenderpass();

		std::vector< vk::raii::Framebuffer > createFrambuffers();

		CameraSwapchain( const vk::Extent2D extent );
	};

} // namespace fgl::engine