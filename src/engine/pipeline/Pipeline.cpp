//
// Created by kj16609 on 11/27/23.
//

#include "Pipeline.hpp"

#include <array>
#include <fstream>

#include "engine/model/Model.hpp"

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
		assert( info.render_pass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no render pass provided" );

		const std::vector< std::byte > vert_data { readFile( vert ) };
		const std::vector< std::byte > frag_data { readFile( frag ) };

		createShaderModule( vert_data, &m_vert_shader );
		createShaderModule( frag_data, &m_frag_shader );

		vk::PipelineShaderStageCreateInfo shaderStages[ 2 ] {};
		shaderStages[ 0 ].pNext = nullptr;
		shaderStages[ 0 ].flags = {};
		shaderStages[ 0 ].stage = vk::ShaderStageFlagBits::eVertex;
		shaderStages[ 0 ].module = m_vert_shader;
		shaderStages[ 0 ].pName = "main";
		shaderStages[ 0 ].pSpecializationInfo = nullptr;

		shaderStages[ 1 ].pNext = nullptr;
		shaderStages[ 1 ].flags = {};
		shaderStages[ 1 ].stage = vk::ShaderStageFlagBits::eFragment;
		shaderStages[ 1 ].module = m_frag_shader;
		shaderStages[ 1 ].pName = "main";
		shaderStages[ 1 ].pSpecializationInfo = nullptr;

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
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shaderStages;
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

	void Pipeline::createShaderModule( const std::vector< std::byte >& code, vk::ShaderModule* module )
	{
		vk::ShaderModuleCreateInfo create_info {};
		create_info.pNext = VK_NULL_HANDLE;
		create_info.flags = {};
		create_info.codeSize = code.size();
		create_info.pCode = reinterpret_cast< const std::uint32_t* >( code.data() );

		if ( m_device.device().createShaderModule( &create_info, nullptr, module ) != vk::Result::eSuccess )
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
		m_device.device().destroyShaderModule( m_vert_shader, nullptr );
		m_device.device().destroyShaderModule( m_frag_shader, nullptr );

		m_device.device().destroyPipelineLayout( m_layout, nullptr );
		m_device.device().destroyPipeline( m_vk_pipeline, nullptr );
	}

	void Pipeline::bind( vk::CommandBuffer command_buffer )
	{
		command_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_vk_pipeline );
	}

} // namespace fgl::engine
