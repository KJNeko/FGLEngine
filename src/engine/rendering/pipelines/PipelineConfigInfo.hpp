//
// Created by kj16609 on 12/14/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <vector>

#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{

	enum VertexInputType
	{
		None,
		Simple,
		Textured
	};

	struct PipelineConfigInfo
	{
		vk::PipelineViewportStateCreateInfo viewport_info {};
		vk::PipelineInputAssemblyStateCreateInfo assembly_info {};
		vk::PipelineTessellationStateCreateInfo tesselation_state_info {};
		vk::PipelineTessellationDomainOriginStateCreateInfo tesselation_domain_info {};
		vk::PipelineRasterizationStateCreateInfo rasterization_info {};
		vk::PipelineMultisampleStateCreateInfo multisample_info {};
		std::vector< vk::PipelineColorBlendAttachmentState > color_blend_attachment {};
		vk::PipelineColorBlendStateCreateInfo color_blend_info {};
		vk::PipelineDepthStencilStateCreateInfo depth_stencil_info {};

		std::vector< vk::DynamicState > dynamic_state_enables {};
		vk::PipelineDynamicStateCreateInfo dynamic_state_info {};

		vk::RenderPass render_pass { VK_NULL_HANDLE };
		std::uint32_t subpass { 0 };

		std::vector< vk::VertexInputBindingDescription > binding_descriptions {};
		std::vector< vk::VertexInputAttributeDescription > attribute_descriptions {};

		FGL_DELETE_COPY( PipelineConfigInfo )

		PipelineConfigInfo( vk::raii::RenderPass& pass );

		PipelineConfigInfo& operator=( PipelineConfigInfo&& other ) = default;
		PipelineConfigInfo( PipelineConfigInfo&& other ) = default;

		static void setVertexInputType( PipelineConfigInfo& info, const VertexInputType type );

		static void disableVertexInput( PipelineConfigInfo& info );

		static void setTriangleListTopo( PipelineConfigInfo& info );
		static void setTriangleStripTopo( PipelineConfigInfo& info );
		static void setLineTopo( PipelineConfigInfo& info );

		static void setQuadTesselation( PipelineConfigInfo& info );
		static void setPointPatch( PipelineConfigInfo& info );
		static void defaultConfig( PipelineConfigInfo& info );
		static void enableAlphaBlending( PipelineConfigInfo& config );
		static void disableCulling( PipelineConfigInfo& info );
		static void addColorAttachmentConfig( PipelineConfigInfo& info );

		static void addGBufferAttachmentsConfig( PipelineConfigInfo& config );
	};

} // namespace fgl::engine