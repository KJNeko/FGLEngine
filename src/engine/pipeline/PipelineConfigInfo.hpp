//
// Created by kj16609 on 12/14/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>

namespace fgl::engine
{

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

		vk::PipelineLayout layout { nullptr };
		vk::RenderPass render_pass { nullptr };
		std::uint32_t subpass { 0 };

		std::vector< vk::VertexInputBindingDescription > binding_descriptions {};
		std::vector< vk::VertexInputAttributeDescription > attribute_descriptions {};

		PipelineConfigInfo( vk::RenderPass pass );
		PipelineConfigInfo( const PipelineConfigInfo& other ) = delete;
		PipelineConfigInfo& operator=( const PipelineConfigInfo& ) = delete;

		static void disableVertexInput( PipelineConfigInfo& info );
		static void setTriangleListTopo( PipelineConfigInfo& info );
		static void setTriangleStripTopo( PipelineConfigInfo& info );
		static void setQuadTesselation( PipelineConfigInfo& info );
		static void setPointPatch( PipelineConfigInfo& info );
		static void defaultConfig( PipelineConfigInfo& info );
		static void enableAlphaBlending( PipelineConfigInfo& config );
		static void addColorAttachmentConfig( PipelineConfigInfo& info );
	};

} // namespace fgl::engine