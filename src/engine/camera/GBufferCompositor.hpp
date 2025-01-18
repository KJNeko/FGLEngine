//
// Created by kj16609 on 1/13/25.
//
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Camera.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/types.hpp"
#include "engine/systems/composition/Control.hpp"

namespace fgl::engine
{
	class CompositeSwapchain;
	class Image;
	class GBufferSwapchain;
	using CompositeFlags = std::uint16_t;

	enum CompositeFlagBits : CompositeFlags
	{
		CompositeFlagBits_None = 0,
		CompositeFlagBits_NormalOnly = 1 << 0,
		//! The final output should be transformed into a presenting valid format.
		CompositeFlagBits_PresentTarget = 1 << 1,

		CompositeFlagBits_Standard = CompositeFlagBits_None,
	};

	/**
	 * @brief Composites a GBuffer input
	 */
	class GBufferCompositor
	{
		CompositeFlags m_flags;

		std::unique_ptr< Pipeline > m_pipeline { nullptr };

		void setViewport( const vk::raii::CommandBuffer& cmd, vk::Extent2D extent_2d );
		void setScissor( const vk::raii::CommandBuffer& cmd, vk::Extent2D extent_2d );

		void beginPass( vk::raii::CommandBuffer& cmd, CompositeSwapchain& swapchain, const FrameIndex& index );
		void endPass( vk::raii::CommandBuffer& cmd );

		CompositionControl m_control {};

	  public:

		GBufferCompositor( CompositeFlags flags = CompositeFlagBits_Standard );

		void composite( vk::raii::CommandBuffer& command_buffer, Camera& camera, FrameIndex frame_index );

		inline void switchMode( const CompositeFlags flags ) { m_flags = flags; }
	};

} // namespace fgl::engine