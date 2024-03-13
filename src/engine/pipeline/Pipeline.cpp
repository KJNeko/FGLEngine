//
// Created by kj16609 on 11/27/23.
//

#include "Pipeline.hpp"

#include <array>
#include <fstream>

#include "Shader.hpp"
#include "engine/model/Model.hpp"

namespace fgl::engine::internal
{

	void Pipeline::createGraphicsPipeline(
		std::vector< std::unique_ptr< ShaderHandle > >& shaders, const PipelineConfigInfo& info )
	{
		assert( info.render_pass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no render pass provided" );

		std::vector< vk::PipelineShaderStageCreateInfo > stages {};

		for ( const auto& shader : shaders )
		{
			stages.emplace_back( shader->stage_info );
		}
		assert( stages.size() >= 2 );

		auto& binding_descriptions { info.binding_descriptions };
		auto& attribute_descriptions { info.attribute_descriptions };

		vk::PipelineVertexInputStateCreateInfo vertex_input_info {};
		vertex_input_info.pNext = VK_NULL_HANDLE;
		vertex_input_info.flags = {};
		vertex_input_info.vertexBindingDescriptionCount = static_cast< std::uint32_t >( binding_descriptions.size() );
		vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
		vertex_input_info.vertexAttributeDescriptionCount =
			static_cast< std::uint32_t >( attribute_descriptions.size() );
		vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

		vk::GraphicsPipelineCreateInfo pipeline_info {};
		pipeline_info.pNext = VK_NULL_HANDLE;
		pipeline_info.flags = {};
		pipeline_info.stageCount = static_cast< std::uint32_t >( stages.size() );
		pipeline_info.pStages = stages.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &info.assembly_info;
		pipeline_info.pTessellationState = VK_NULL_HANDLE;
		pipeline_info.pViewportState = &info.viewport_info;
		pipeline_info.pRasterizationState = &info.rasterization_info;
		pipeline_info.pMultisampleState = &info.multisample_info;
		pipeline_info.pDepthStencilState = &info.depth_stencil_info;
		pipeline_info.pColorBlendState = &info.color_blend_info;
		pipeline_info.pDynamicState = &info.dynamic_state_info;
		pipeline_info.layout = info.layout;
		pipeline_info.renderPass = info.render_pass;
		pipeline_info.subpass = info.subpass;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		if ( auto temp = m_device.device().createGraphicsPipeline( VK_NULL_HANDLE, pipeline_info );
		     temp.result != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to create graphics pipeline" );
		else
			m_vk_pipeline = temp.value;
	}

	Pipeline::~Pipeline()
	{
		m_device.device().destroyPipelineLayout( m_layout, nullptr );
		m_device.device().destroyPipeline( m_vk_pipeline, nullptr );
	}

	void Pipeline::bind( vk::CommandBuffer command_buffer )
	{
		command_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_vk_pipeline );
	}

} // namespace fgl::engine::internal
