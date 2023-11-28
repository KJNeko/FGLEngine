//
// Created by kj16609 on 11/27/23.
//

#include "Pipeline.hpp"

#include <array>
#include <fstream>

#include "Model.hpp"

namespace fgl::engine
{

	std::vector< std::byte > Pipeline::readFile( const std::filesystem::path& path )
	{
		if ( std::ifstream ifs( path, std::ios::binary | std::ios::ate ); ifs )
		{
			std::vector< std::byte > data;

			data.resize( static_cast< std::size_t >( ifs.tellg() ) );
			ifs.seekg( 0, std::ios::beg );

			ifs.read( reinterpret_cast< char* >( data.data() ), static_cast< long >( data.size() ) );

			return data;
		}
		else
			throw std::runtime_error( "Failed to load file: " + path.string() );
	}

	void Pipeline::createGraphicsPipeline(
		const std::filesystem::path& vert, const std::filesystem::path& frag, const PipelineConfigInfo& info )
	{
		const std::vector< std::byte > vert_data { readFile( vert ) };
		const std::vector< std::byte > frag_data { readFile( frag ) };

		createShaderModule( vert_data, &m_vert_shader );
		createShaderModule( frag_data, &m_frag_shader );

		VkPipelineShaderStageCreateInfo shaderStages[ 2 ] {
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = m_vert_shader,
				.pName = "main",
				.pSpecializationInfo = nullptr,
			},
			{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			  .pNext = nullptr,
			  .flags = 0,
			  .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			  .module = m_frag_shader,
			  .pName = "main",
			  .pSpecializationInfo = nullptr }

		};

		auto binding_descriptions { Model::Vertex::getBindingDescriptions() };
		auto attribute_descriptions { Model::Vertex::getAttributeDescriptions() };

		VkPipelineVertexInputStateCreateInfo vertex_input_info {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = static_cast< std::uint32_t >( binding_descriptions.size() ),
			.pVertexBindingDescriptions = binding_descriptions.data(),
			.vertexAttributeDescriptionCount = static_cast< std::uint32_t >( attribute_descriptions.size() ),
			.pVertexAttributeDescriptions = attribute_descriptions.data(),
		};

		VkGraphicsPipelineCreateInfo pipeline_info {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = 2,
			.pStages = shaderStages,
			.pVertexInputState = &vertex_input_info,
			.pInputAssemblyState = &info.assembly_info,
			.pViewportState = &info.viewport_info,
			.pRasterizationState = &info.rasterization_info,
			.pMultisampleState = &info.multisample_info,
			.pDepthStencilState = &info.depth_stencil_info,
			.pColorBlendState = &info.color_blend_info,
			.pDynamicState = &info.dynamic_state_info,

			.layout = info.layout,
			.renderPass = info.render_pass,
			.subpass = info.subpass,

			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1,
		};

		if ( vkCreateGraphicsPipelines( m_device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_vk_pipeline )
		     != VK_SUCCESS )
			throw std::runtime_error( "Failed to create graphics pipeline" );
	}

	void Pipeline::createShaderModule( const std::vector< std::byte >& code, VkShaderModule* module )
	{
		VkShaderModuleCreateInfo create_info { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			                                   .codeSize = code.size(),
			                                   .pCode = reinterpret_cast< const std::uint32_t* >( code.data() ) };

		if ( vkCreateShaderModule( m_device.device(), &create_info, nullptr, module ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create shader module" );
	}

	Pipeline::Pipeline(
		Device& device,
		const std::filesystem::path& vert,
		const std::filesystem::path& frag,
		const PipelineConfigInfo& info ) :
	  m_device( device )
	{
		createGraphicsPipeline( vert, frag, info );
	}

	Pipeline::~Pipeline()
	{
		vkDestroyShaderModule( m_device.device(), m_vert_shader, nullptr );
		vkDestroyShaderModule( m_device.device(), m_frag_shader, nullptr );

		vkDestroyPipeline( m_device.device(), m_vk_pipeline, nullptr );
	}

	void Pipeline::defaultConfig( PipelineConfigInfo& info )
	{
		info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		info.viewport_info.viewportCount = 1;
		info.viewport_info.pViewports = nullptr;
		info.viewport_info.scissorCount = 1;
		info.viewport_info.pScissors = nullptr;

		info.assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		info.assembly_info.primitiveRestartEnable = VK_FALSE;

		info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.rasterization_info.depthClampEnable = VK_FALSE;
		info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
		info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
		info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
		info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		info.rasterization_info.depthBiasEnable = VK_FALSE;
		info.rasterization_info.depthBiasConstantFactor = 0.0f;
		info.rasterization_info.depthBiasClamp = 0.0f;
		info.rasterization_info.depthBiasSlopeFactor = 0.0f;
		info.rasterization_info.lineWidth = 1.0f;

		info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		info.multisample_info.sampleShadingEnable = VK_FALSE;
		info.multisample_info.minSampleShading = 1.0f;
		info.multisample_info.pSampleMask = nullptr;
		info.multisample_info.alphaToCoverageEnable = VK_FALSE;
		info.multisample_info.alphaToOneEnable = VK_FALSE;

		info.color_blend_attachment.blendEnable = VK_FALSE;
		info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		info.color_blend_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.color_blend_info.logicOpEnable = VK_FALSE;
		info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
		info.color_blend_info.attachmentCount = 1;
		info.color_blend_info.pAttachments = &info.color_blend_attachment;
		info.color_blend_info.blendConstants[ 0 ] = 0.0f;
		info.color_blend_info.blendConstants[ 1 ] = 0.0f;
		info.color_blend_info.blendConstants[ 2 ] = 0.0f;
		info.color_blend_info.blendConstants[ 3 ] = 0.0f;

		info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.depth_stencil_info.depthTestEnable = VK_TRUE;
		info.depth_stencil_info.depthWriteEnable = VK_TRUE;
		info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
		info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		info.depth_stencil_info.stencilTestEnable = VK_FALSE;
		info.depth_stencil_info.front = {};
		info.depth_stencil_info.back = {};
		info.depth_stencil_info.minDepthBounds = 0.0f;
		info.depth_stencil_info.maxDepthBounds = 1.0f;

		info.dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		info.dynamic_state_info.pDynamicStates = info.dynamic_state_enables.data();
		info.dynamic_state_info.dynamicStateCount = static_cast< std::uint32_t >( info.dynamic_state_enables.size() );
		info.dynamic_state_info.flags = 0;
	}

	void Pipeline::bind( VkCommandBuffer command_buffer )
	{
		vkCmdBindPipeline( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vk_pipeline );
	}

} // namespace fgl::engine
