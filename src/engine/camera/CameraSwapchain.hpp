//
// Created by kj16609 on 7/11/24.
//

#pragma once

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/rendering/RenderingFormats.hpp"
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
			ColorAttachment< COLOR_INDEX > m_color { pickColorFormat() };
			ColorAttachment< POSITION_INDEX > m_position { pickPositionFormat() };
			ColorAttachment< NORMAL_INDEX > m_normal { pickNormalFormat() };
			ColorAttachment< METALLIC_INDEX > m_metallic { pickMetallicFormat() };
			ColorAttachment< EMISSIVE_INDEX > m_emissive { pickEmissiveFormat() };

			ColorAttachment< COMPOSITE_INDEX > m_composite { pickCompositeFormat() };
			DepthAttachment< DEPTH_INDEX > m_depth { pickDepthFormat() };
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

		enum StageID : std::uint16_t
		{
			INITAL = 0,
			FINAL = std::numeric_limits< std::uint16_t >::max()
		};

		void transitionImages( vk::raii::CommandBuffer& command_buffer, std::uint16_t stage_id, FrameIndex index );
		vk::RenderingInfo getRenderingInfo( const FrameIndex frame_index );

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