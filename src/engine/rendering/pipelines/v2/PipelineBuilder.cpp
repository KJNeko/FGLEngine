//
// Created by kj16609 on 10/10/24.
//

#include "PipelineBuilder.hpp"

#include "AttachmentBuilder.hpp"
#include "Pipeline.hpp"
#include "engine/descriptors/DescriptorSetLayout.hpp"
#include "engine/rendering/PresentSwapChain.hpp"
#include "engine/rendering/RenderingFormats.hpp"

namespace fgl::engine
{

	PipelineBuilder::PipelineBuilder( std::uint32_t subpass ) : m_state( std::make_unique< BuilderState >( subpass ) )
	{
		addDynamicState( vk::DynamicState::eViewport );
		addDynamicState( vk::DynamicState::eScissor );
	}

	void PipelineBuilder::setVertexShader( std::shared_ptr< Shader >&& shader )
	{
		m_state->shaders.vertex = std::forward< std::shared_ptr< Shader > >( shader );
	}

	void PipelineBuilder::setFragmentShader( std::shared_ptr< Shader >&& shader )
	{
		m_state->shaders.fragment = std::forward< std::shared_ptr< Shader > >( shader );
	}

	void PipelineBuilder::setComputeShader( std::shared_ptr< Shader >&& shader )
	{
		m_state->shaders.compute = std::forward< std::shared_ptr< Shader > >( shader );
	}

	vk::raii::Pipeline PipelineBuilder::
		createRenderPassPipeline( BuilderState& state, const vk::raii::PipelineLayout& layout )
	{
		FGL_UNREACHABLE();

		vk::GraphicsPipelineCreateInfo info {};
		info.pNext = VK_NULL_HANDLE;
		info.flags = {};

		std::vector< vk::PipelineShaderStageCreateInfo > stages {};

		if ( state.shaders.vertex ) stages.emplace_back( state.shaders.vertex->stage_info );
		if ( state.shaders.fragment ) stages.emplace_back( state.shaders.fragment->stage_info );

		info.setStages( stages );

		vk::PipelineVertexInputStateCreateInfo vertex_input_info {};
		vertex_input_info.pNext = VK_NULL_HANDLE;
		vertex_input_info.flags = {};
		vertex_input_info.setVertexBindingDescriptions( state.vertex_input_descriptions.bindings );
		vertex_input_info.setVertexAttributeDescriptions( state.vertex_input_descriptions.attributes );
		info.setPVertexInputState( &vertex_input_info );

		info.pInputAssemblyState = &state.assembly_info;
		info.pTessellationState = &state.tesselation_state_info;
		info.pViewportState = &state.viewport_info;
		info.pRasterizationState = &state.rasterization_info;
		info.pMultisampleState = &state.multisample_info;
		info.pDepthStencilState = &state.depth_stencil_info;

		state.color_blend_info.setAttachments( state.color_blend_attachment );

		info.pColorBlendState = &state.color_blend_info;
		info.pDynamicState = &state.dynamic_state_info;

		info.layout = layout;

		info.subpass = state.m_subpass_stage;

		//TODO: Figure out what these do
		info.basePipelineHandle = VK_NULL_HANDLE;
		info.basePipelineIndex = -1;

		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info {};
		dynamic_state_create_info.setDynamicStates( state.m_dynamic_state );

		if ( state.m_dynamic_state.size() > 0 ) info.setPDynamicState( &dynamic_state_create_info );

		vk::raii::Pipeline pipeline { Device::getInstance()->createGraphicsPipeline( VK_NULL_HANDLE, info ) };

		return pipeline;
	}

	vk::raii::Pipeline PipelineBuilder::createComputePipeline( BuilderState& state, vk::raii::PipelineLayout& layout )
	{
		vk::StructureChain< vk::ComputePipelineCreateInfo > chain {};

		vk::ComputePipelineCreateInfo& info { chain.get< vk::ComputePipelineCreateInfo >() };

		info.pNext = VK_NULL_HANDLE;
		info.flags = {};

		info.stage = state.shaders.compute->stage_info;
		info.layout = layout;
		info.basePipelineHandle = VK_NULL_HANDLE;
		info.basePipelineIndex = -1;

		state.bind_point = vk::PipelineBindPoint::eCompute;

		return Device::getInstance()->createComputePipeline( VK_NULL_HANDLE, info );
	}

	vk::raii::Pipeline PipelineBuilder::createDynamicPipeline( BuilderState& state, vk::raii::PipelineLayout& layout )
	{
		if ( state.shaders.compute ) return createComputePipeline( state, layout );
		return createGraphicsPipeline( state, layout );
	}

	vk::raii::Pipeline PipelineBuilder::createGraphicsPipeline( BuilderState& state, vk::raii::PipelineLayout& layout )
	{
		vk::StructureChain< vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo > chain {};
		vk::GraphicsPipelineCreateInfo& info { chain.get< vk::GraphicsPipelineCreateInfo >() };
		info.pNext = VK_NULL_HANDLE;
		info.flags = {};

		chain.relink< vk::PipelineRenderingCreateInfo >();

		std::vector< vk::PipelineShaderStageCreateInfo > stages {};

		if ( state.shaders.vertex ) stages.emplace_back( state.shaders.vertex->stage_info );
		if ( state.shaders.fragment ) stages.emplace_back( state.shaders.fragment->stage_info );

		info.setStages( stages );

		vk::PipelineVertexInputStateCreateInfo vertex_input_info {};
		vertex_input_info.pNext = VK_NULL_HANDLE;
		vertex_input_info.flags = {};
		vertex_input_info.setVertexBindingDescriptions( state.vertex_input_descriptions.bindings );
		vertex_input_info.setVertexAttributeDescriptions( state.vertex_input_descriptions.attributes );
		info.setPVertexInputState( &vertex_input_info );

		info.pInputAssemblyState = &state.assembly_info;
		info.pTessellationState = &state.tesselation_state_info;
		info.pViewportState = &state.viewport_info;
		info.pRasterizationState = &state.rasterization_info;
		info.pMultisampleState = &state.multisample_info;
		info.pDepthStencilState = &state.depth_stencil_info;

		state.color_blend_info.setAttachments( state.color_blend_attachment );

		info.pColorBlendState = &state.color_blend_info;
		info.pDynamicState = &state.dynamic_state_info;

		info.layout = layout;

		info.subpass = state.m_subpass_stage;

		//TODO: Figure out what these do
		info.basePipelineHandle = VK_NULL_HANDLE;
		info.basePipelineIndex = -1;

		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info {};
		dynamic_state_create_info.setDynamicStates( state.m_dynamic_state );

		vk::PipelineRenderingCreateInfo& rendering_info { chain.get< vk::PipelineRenderingCreateInfo >() };

		rendering_info.setColorAttachmentFormats( state.formats.colors );

		if ( state.formats.depth != vk::Format::eUndefined )
			rendering_info.setDepthAttachmentFormat( state.formats.depth );

		if ( state.m_dynamic_state.size() > 0 ) info.setPDynamicState( &dynamic_state_create_info );

		state.bind_point = vk::PipelineBindPoint::eGraphics;

		vk::raii::Pipeline pipeline { Device::getInstance()->createGraphicsPipeline( VK_NULL_HANDLE, info ) };

		return pipeline;
	}

	vk::raii::PipelineLayout PipelineBuilder::createLayout()
	{
		vk::PipelineLayoutCreateInfo info {};

		if ( m_state->push_constant.size > 0 ) info.setPushConstantRanges( m_state->push_constant );

		std::vector< vk::DescriptorSetLayout > set_layouts {};

		set_layouts.reserve( m_state->descriptor_set_layouts.size() );

		SetID max_set_idx { 0 };

		for ( const auto& [ set_idx, _ ] : m_state->descriptor_set_layouts )
		{
			max_set_idx = std::max( max_set_idx, set_idx );
		}

		// Any sets not used, Should be set to VK_NULL_HANDLE
		set_layouts.resize( max_set_idx + 1 );

		for ( std::size_t i = 0; i < set_layouts.size(); ++i )
		{
			auto itter { m_state->descriptor_set_layouts.find( static_cast< SetID >( i ) ) };
			if ( itter == m_state->descriptor_set_layouts.end() )
			{
				// Could not find it. Empty
				set_layouts[ i ] = m_empty_set_layout.layout();
			}
			else
			{
				set_layouts[ i ] = itter->second;
			}
		}

		for ( const auto& [ set_idx, layout ] : m_state->descriptor_set_layouts )
		{
			set_layouts[ set_idx ] = layout;
		}

		info.setSetLayouts( set_layouts );

		return Device::getInstance()->createPipelineLayout( info );
	}

	void PipelineBuilder::
		addDescriptorSet( const SetID idx, const vk::raii::DescriptorSetLayout& descriptor_set_layout ) const
	{
		FGL_ASSERT( !m_state->descriptor_set_layouts.contains( idx ), "Descriptor already set!" );
		m_state->descriptor_set_layouts.insert( std::make_pair( idx, *descriptor_set_layout ) );
	}

	void PipelineBuilder::addDescriptorSet( descriptors::DescriptorSetLayout& descriptor )
	{
		addDescriptorSet( descriptor.m_set_idx, descriptor.layout() );
	}

	void PipelineBuilder::addDynamicState( vk::DynamicState dynamic_state )
	{
		m_state->m_dynamic_state.emplace_back( dynamic_state );
	}

	void PipelineBuilder::setPushConstant( const vk::ShaderStageFlags flags, std::uint32_t size )
	{
		m_state->push_constant.offset = 0;
		m_state->push_constant.size = size;
		m_state->push_constant.stageFlags = flags;
	}

	void PipelineBuilder::setBindPoint( vk::PipelineBindPoint bind_point )
	{
		m_state->bind_point = bind_point;
	}

	PipelineBuilder::BuilderState::Formats::Formats()
	{}

	[[nodiscard]] vk::PipelineColorBlendAttachmentState& PipelineBuilder::BuilderState::addColorAttachment()
	{
		color_blend_attachment.emplace_back();
		color_blend_info.setAttachments( color_blend_attachment );
		return color_blend_attachment.back();
	}

	void PipelineBuilder::BuilderState::setDefault()
	{
		viewport_info.viewportCount = 1;
		viewport_info.pViewports = nullptr;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = nullptr;

		assembly_info.topology = vk::PrimitiveTopology::eTriangleList;
		assembly_info.primitiveRestartEnable = VK_FALSE;

		rasterization_info.depthClampEnable = VK_FALSE;
		rasterization_info.rasterizerDiscardEnable = VK_FALSE;
		rasterization_info.polygonMode = vk::PolygonMode::eFill;
		rasterization_info.cullMode = vk::CullModeFlagBits::eBack;
		rasterization_info.frontFace = vk::FrontFace::eClockwise;
		rasterization_info.depthBiasEnable = VK_FALSE;
		rasterization_info.depthBiasConstantFactor = 0.0f;
		rasterization_info.depthBiasClamp = 0.0f;
		rasterization_info.depthBiasSlopeFactor = 0.0f;
		rasterization_info.lineWidth = 1.0f;

		multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisample_info.sampleShadingEnable = VK_FALSE;
		multisample_info.minSampleShading = 1.0f;
		multisample_info.pSampleMask = nullptr;
		multisample_info.alphaToCoverageEnable = VK_FALSE;
		multisample_info.alphaToOneEnable = VK_FALSE;

		color_blend_info.logicOpEnable = VK_FALSE;
		color_blend_info.logicOp = vk::LogicOp::eCopy;
		color_blend_info.attachmentCount = 0;
		color_blend_info.pAttachments = nullptr;
		color_blend_info.blendConstants[ 0 ] = 0.0f;
		color_blend_info.blendConstants[ 1 ] = 0.0f;
		color_blend_info.blendConstants[ 2 ] = 0.0f;
		color_blend_info.blendConstants[ 3 ] = 0.0f;

		depth_stencil_info.depthTestEnable = VK_TRUE;
		depth_stencil_info.depthWriteEnable = VK_TRUE;
		depth_stencil_info.depthCompareOp = vk::CompareOp::eLess;
		depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_info.stencilTestEnable = VK_FALSE;
		//depth_stencil_info.front = {};
		//depth_stencil_info.back = {};
		depth_stencil_info.minDepthBounds = 0.0f;
		depth_stencil_info.maxDepthBounds = 1.0f;

		dynamic_state_enables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		dynamic_state_info.setDynamicStates( dynamic_state_enables );
		//info.dynamic_state_info.flags = 0;
	}

	PipelineBuilder::BuilderState::BuilderState( std::uint32_t subpass ) : m_subpass_stage( subpass )
	{
		setDefault();
	}

	void PipelineBuilder::setTopology( const vk::PrimitiveTopology primitive_topology )
	{
		m_state->assembly_info.topology = primitive_topology;
	}

	void PipelineBuilder::disableVertexInput()
	{
		m_state->vertex_input_descriptions.bindings = {};
		m_state->vertex_input_descriptions.attributes = {};
	}

	void PipelineBuilder::disableCulling()
	{
		m_state->rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
	}

	void PipelineBuilder::addDepthAttachment()
	{
		m_state->formats.depth = pickDepthFormat();
	}

	AttachmentBuilder PipelineBuilder::addAttachment()
	{
		return { *this };
	}

	AttachmentBuilder PipelineBuilder::addColorAttachment()
	{
		AttachmentBuilder builder { addAttachment() };
		return builder;
	}

	void PipelineBuilder::setBindingDescriptions( const std::vector< vk::VertexInputBindingDescription >& descriptions )
	{
		m_state->vertex_input_descriptions.bindings = descriptions;
	}

	void PipelineBuilder::
		setAttributeDescriptions( const std::vector< vk::VertexInputAttributeDescription >& descriptions ) const
	{
		m_state->vertex_input_descriptions.attributes = descriptions;
	}

	vk::raii::Pipeline PipelineBuilder::createFromState( BuilderState& state, vk::raii::PipelineLayout& layout )
	{
		return createDynamicPipeline( state, layout );
	}

	vk::raii::Pipeline PipelineBuilder::rebuildFromState( BuilderState& state, vk::raii::PipelineLayout& layout )
	{
		auto& shaders { state.shaders };

		if ( shaders.vertex ) shaders.vertex->reload();
		if ( shaders.fragment ) shaders.fragment->reload();

		return createFromState( state, layout );
	}

	std::unique_ptr< Pipeline > PipelineBuilder::create()
	{
		// Precheck
		{
			// FGL_ASSERT( m_state->shaders.fragment, "Pipeline requires fragment shader" );
			// FGL_ASSERT( m_state->shaders.vertex, "Pipeline requires vertex shader" );
		}

		vk::raii::PipelineLayout layout { createLayout() };

		vk::raii::Pipeline pipeline { createFromState( *m_state, layout ) };

		return std::make_unique<
			Pipeline >( std::move( pipeline ), std::move( layout ), m_state->bind_point, std::move( m_state ) );
	}

	void setGBufferOutputAttachments( PipelineBuilder::BuilderState& config )
	{
		// In order for the pipeline to output, We need to ensure that we have enough attachments for the entire gbuffer (3)

		FGL_ASSERT( config.color_blend_attachment.size() == 0, "GBuffer output expected there to be no other outputs" );

		for ( int i = 0; i < 3; ++i )
		{
			auto& color_config { config.addColorAttachment() };

			color_config.blendEnable = VK_FALSE;
			color_config.srcColorBlendFactor = vk::BlendFactor::eOne;
			color_config.dstColorBlendFactor = vk::BlendFactor::eZero;
			color_config.colorBlendOp = vk::BlendOp::eAdd;
			color_config.srcAlphaBlendFactor = vk::BlendFactor::eOne;
			color_config.dstAlphaBlendFactor = vk::BlendFactor::eZero;
			color_config.alphaBlendOp = vk::BlendOp::eAdd;
			color_config.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
			                            | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		}
	}

	void addGBufferAttachments( PipelineBuilder& builder )
	{
		builder.addColorAttachment().setFormat( pickColorFormat() ).finish();
		builder.addColorAttachment().setFormat( pickPositionFormat() ).finish();
		builder.addColorAttachment().setFormat( pickNormalFormat() ).finish();
		builder.addColorAttachment().setFormat( pickMetallicFormat() ).finish();
		builder.addColorAttachment().setFormat( pickEmissiveFormat() ).finish();
	}

} // namespace fgl::engine
