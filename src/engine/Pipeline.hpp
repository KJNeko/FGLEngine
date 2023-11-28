//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include <filesystem>
#include <vector>

#include "Device.hpp"

namespace fgl::engine
{
	struct PipelineConfigInfo
	{
		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineInputAssemblyStateCreateInfo assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;

		std::vector< VkDynamicState > dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;

		VkPipelineLayout layout { nullptr };
		VkRenderPass render_pass { nullptr };
		std::uint32_t subpass { 0 };

		PipelineConfigInfo() = default;
		PipelineConfigInfo( const PipelineConfigInfo& other ) = delete;
		PipelineConfigInfo& operator=( const PipelineConfigInfo& ) = delete;
	};

	class Pipeline
	{
		Device& m_device;
		VkPipeline m_vk_pipeline;
		VkShaderModule m_vert_shader;
		VkShaderModule m_frag_shader;

		static std::vector< std::byte > readFile( const std::filesystem::path& path );

		void createGraphicsPipeline(
			const std::filesystem::path& vert, const std::filesystem::path& frag, const PipelineConfigInfo& info );

		void createShaderModule( const std::vector< std::byte >& code, VkShaderModule* module );

	  public:

		void bind( VkCommandBuffer command_buffer );

		Pipeline(
			Device& device,
			const std::filesystem::path& vert,
			const std::filesystem::path& frag,
			const PipelineConfigInfo& info );

		static void defaultConfig( PipelineConfigInfo& info );

		~Pipeline();

		Pipeline( const Pipeline& other ) = delete;
		Pipeline& operator=( const Pipeline& ) = delete;
	};
} // namespace fgl::engine