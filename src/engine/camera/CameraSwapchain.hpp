//
// Created by kj16609 on 7/11/24.
//

#pragma once

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/rendering/pipelines/Attachment.hpp"

namespace fgl::engine
{

	constexpr std::size_t COLOR_INDEX { 0 };
	constexpr std::size_t POSITION_INDEX { 1 };
	constexpr std::size_t NORMAL_INDEX { 2 };
	constexpr std::size_t METALLIC_INDEX { 3 };
	constexpr std::size_t EMISSIVE_INDEX { 4 };

	constexpr std::size_t COMPOSITE_INDEX { 5 };
	constexpr std::size_t DEPTH_INDEX { 6 };

	class CameraSwapchain
	{
		struct
		{
			ColorAttachment< COLOR_INDEX > m_color { vk::Format::eR8G8B8A8Unorm };
			ColorAttachment< POSITION_INDEX > m_position { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< NORMAL_INDEX > m_normal { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< METALLIC_INDEX > m_metallic { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< EMISSIVE_INDEX > m_emissive { vk::Format::eR16G16B16A16Sfloat };

			ColorAttachment< COMPOSITE_INDEX > m_composite { vk::Format::eR8G8B8A8Unorm };
			DepthAttachment< DEPTH_INDEX > m_depth { SwapChain::findDepthFormat() };
		} m_gbuffer {};

	  public:

		std::vector< std::unique_ptr< Texture > > m_g_buffer_color_img {};
		std::vector< std::unique_ptr< Texture > > m_g_buffer_position_img {};
		std::vector< std::unique_ptr< Texture > > m_g_buffer_normal_img {};
		std::vector< std::unique_ptr< Texture > > m_g_buffer_metallic_img {};
		std::vector< std::unique_ptr< Texture > > m_g_buffer_emissive_img {};

		std::vector< std::unique_ptr< Texture > > m_g_buffer_composite_img {};

	  private:

		vk::Extent2D m_extent;

		vk::raii::RenderPass& m_renderpass;

		std::vector< vk::raii::Framebuffer > m_framebuffers;

		std::vector< vk::ClearValue > m_clear_values;

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > m_gbuffer_descriptor_set {};

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > createGBufferDescriptors();

	  public:

		CameraSwapchain( vk::raii::RenderPass& renderpass, vk::Extent2D extent );
		~CameraSwapchain();

		const std::vector< vk::ClearValue >& getClearValues();

		std::vector< vk::raii::Framebuffer > createFrambuffers();

		descriptors::DescriptorSet& getGBufferDescriptor( FrameIndex frame_index );

		vk::raii::Framebuffer& getFramebuffer( FrameIndex frame_index );

		vk::Extent2D getExtent() const;

		Image& getOutput( const FrameIndex index );

		float getAspectRatio();
	};

} // namespace fgl::engine