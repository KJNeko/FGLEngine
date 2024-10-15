//
// Created by kj16609 on 7/11/24.
//

#pragma once

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/rendering/pipelines/Attachment.hpp"

namespace fgl::engine
{

	constexpr std::size_t ColorIndex { 0 };
	constexpr std::size_t PositionIndex { 1 };
	constexpr std::size_t NormalIndex { 2 };
	constexpr std::size_t MetallicIndex { 3 };
	constexpr std::size_t EmissiveIndex { 4 };

	constexpr std::size_t CompositeIndex { 5 };
	constexpr std::size_t DepthIndex { 6 };

	class CameraSwapchain
	{
		struct
		{
			ColorAttachment< ColorIndex > color { vk::Format::eR8G8B8A8Unorm };
			ColorAttachment< PositionIndex > position { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< NormalIndex > normal { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< MetallicIndex > metallic { vk::Format::eR16G16B16A16Sfloat };
			ColorAttachment< EmissiveIndex > emissive { vk::Format::eR16G16B16A16Sfloat };

			ColorAttachment< CompositeIndex > composite { vk::Format::eR8G8B8A8Unorm };
			DepthAttachment< DepthIndex > depth { SwapChain::findDepthFormat() };
		} gbuffer {};

	  public:

		std::vector< std::unique_ptr< Texture > > g_buffer_color_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_position_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_normal_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_metallic_img {};
		std::vector< std::unique_ptr< Texture > > g_buffer_emissive_img {};

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