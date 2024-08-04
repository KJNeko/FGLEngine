//
// Created by kj16609 on 7/11/24.
//

#pragma once

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/rendering/Attachment.hpp"
#include "engine/rendering/SwapChain.hpp"

namespace fgl::engine
{

	class CameraSwapchain
	{
		struct
		{
			ColorAttachment< 0 > position { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< 1 > normal { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< 2 > albedo { vk::Format::eR8G8B8A8Unorm };
			ColorAttachment< 3 > composite { vk::Format::eR8G8B8A8Unorm };
			DepthAttachment< 4 > depth { SwapChain::findDepthFormat() };
		} gbuffer {};

	  public:

		std::vector< std::unique_ptr< Texture > > g_buffer_position_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_normal_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_albedo_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_composite_img {};

	  private:

		vk::Extent2D m_extent;

		vk::raii::RenderPass& m_renderpass;

		std::vector< vk::raii::Framebuffer > m_framebuffers;

		std::vector< vk::ClearValue > m_clear_values;

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > m_gbuffer_descriptor_set {};

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > createGBufferDescriptors();

	  public:

		CameraSwapchain( vk::raii::RenderPass& renderpass, vk::Extent2D extent );

		const std::vector< vk::ClearValue >& getClearValues();

		std::vector< vk::raii::Framebuffer > createFrambuffers();

		descriptors::DescriptorSet& getGBufferDescriptor( FrameIndex frame_index );

		vk::raii::Framebuffer& getFramebuffer( FrameIndex frame_index );

		vk::Extent2D getExtent() const;

		Image& getOutput( const FrameIndex index );

		float getAspectRatio();
	};

} // namespace fgl::engine