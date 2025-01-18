//
// Created by kj16609 on 1/13/25.
//
#pragma once
#include "engine/rendering/RenderingFormats.hpp"
#include "engine/rendering/pipelines/Attachment.hpp"

namespace fgl::engine
{
	class Texture;

	class CompositeSwapchain
	{
		vk::Extent2D m_extent;

		struct
		{
			ColorAttachment< 0 > m_target { pickCompositeFormat() };
		} m_buffer {};

	  public:

		FGL_DELETE_COPY( CompositeSwapchain );
		FGL_DELETE_MOVE( CompositeSwapchain );

		std::vector< std::unique_ptr< Texture > > m_gbuffer_target {};

		enum StageID : std::uint16_t
		{
			INITAL,
			FINAL,
		};

		void transitionImages( vk::raii::CommandBuffer& command_buffer, StageID stage_id, FrameIndex index );

		vk::RenderingInfo getRenderingInfo( FrameIndex index );

		vk::Extent2D getExtent() const { return m_extent; }

		CompositeSwapchain( vk::Extent2D extent );
		~CompositeSwapchain();
	};

} // namespace fgl::engine