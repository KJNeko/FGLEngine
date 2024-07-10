//
// Created by kj16609 on 7/10/24.
//

#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Attachment.hpp"

/*
namespace fgl::engine
{
	class PhysicalDevice;

	vk::Format pickOptimalDepthFormat();

	class CameraSwapchain
	{
		PhysicalDevice& m_physical_device;

		//! Contains the information for the final color buffer
		struct
		{
			vk::Format m_color_format;
			vk::Format m_depth_format;
		} m_final_render_info;

		struct
		{
			// Final output (composite) attachments
			ColorAttachment color_attachment;
			DepthAttachment depth_attachment;

			// GBuffer Attachments
			ColorAttachment position_attachment { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment normal_attachment { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment albedo_attachment { vk::Format::eR8G8B8A8Unorm };
		} m_attachments;

		vk::Extent2D m_extent;

		vk::raii::RenderPass m_renderpass;

		//! Framebuffers used in each frame (index using frame_idx)
		std::vector< vk::raii::Framebuffer > m_framebuffers;

		struct
		{
			//! Triggered when the image is ready to be used again
			std::vector< vk::raii::Semaphore > image_available;

			//! Triggered when the frame is finished rendering
			std::vector< vk::raii::Semaphore > render_finished;
		} semaphores;

		std::vector< vk::raii::Framebuffer > createFramebuffers();

		vk::raii::RenderPass createRenderpass();

	  public:

		CameraSwapchain( PhysicalDevice& physical_device, vk::Extent2D starting_extent );
	};
} // namespace fgl::engine
*/