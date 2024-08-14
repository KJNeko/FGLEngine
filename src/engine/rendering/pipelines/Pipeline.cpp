//
// Created by kj16609 on 11/27/23.
//

#include "Pipeline.hpp"

#include <fstream>

#include "Shader.hpp"
#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine::internal
{

	vk::raii::Pipeline Pipeline::createGraphicsPipeline(
		std::vector< std::unique_ptr< ShaderHandle > >& shaders,
		const PipelineConfigInfo& info,
		const vk::raii::PipelineLayout& layout )
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
		pipeline_info.pTessellationState = &info.tesselation_state_info;
		pipeline_info.pViewportState = &info.viewport_info;
		pipeline_info.pRasterizationState = &info.rasterization_info;
		pipeline_info.pMultisampleState = &info.multisample_info;
		pipeline_info.pDepthStencilState = &info.depth_stencil_info;
		pipeline_info.pColorBlendState = &info.color_blend_info;
		pipeline_info.pDynamicState = &info.dynamic_state_info;
		pipeline_info.layout = *layout;
		pipeline_info.renderPass = info.render_pass;
		pipeline_info.subpass = info.subpass;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		return m_device->createGraphicsPipeline( VK_NULL_HANDLE, pipeline_info );
	}

	Pipeline::Pipeline(
		Device& device,
		vk::raii::PipelineLayout layout,
		PipelineConfigInfo info,
		std::vector< std::unique_ptr< ShaderHandle > > shaders ) :
	  m_device( device ),
	  m_layout( std::move( layout ) ),
	  m_vk_pipeline( createGraphicsPipeline( shaders, info, m_layout ) )
	{}

	void Pipeline::bind( vk::raii::CommandBuffer& command_buffer )
	{
		command_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_vk_pipeline );
	}

	void Pipeline::setDebugName( const std::string str )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::ePipeline;
		info.pObjectName = str.c_str();
		info.objectHandle = reinterpret_cast< std::uint64_t >( static_cast< VkPipeline >( *this->m_vk_pipeline ) );
		Device::getInstance().setDebugUtilsObjectName( info );
	}

} // namespace fgl::engine::internal
