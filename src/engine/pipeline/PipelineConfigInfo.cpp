//
// Created by kj16609 on 12/14/23.
//

#include "PipelineConfigInfo.hpp"

#include "engine/model/Model.hpp"

namespace fgl::engine
{

	void PipelineConfigInfo::defaultConfig( PipelineConfigInfo& info )
	{
		info.viewport_info.viewportCount = 1;
		info.viewport_info.pViewports = nullptr;
		info.viewport_info.scissorCount = 1;
		info.viewport_info.pScissors = nullptr;

		info.assembly_info.topology = vk::PrimitiveTopology::eTriangleList;
		info.assembly_info.primitiveRestartEnable = VK_FALSE;

		info.rasterization_info.depthClampEnable = VK_FALSE;
		info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
		info.rasterization_info.polygonMode = vk::PolygonMode::eFill;
		info.rasterization_info.cullMode = vk::CullModeFlagBits::eBack;
		info.rasterization_info.frontFace = vk::FrontFace::eClockwise;
		info.rasterization_info.depthBiasEnable = VK_FALSE;
		info.rasterization_info.depthBiasConstantFactor = 0.0f;
		info.rasterization_info.depthBiasClamp = 0.0f;
		info.rasterization_info.depthBiasSlopeFactor = 0.0f;
		info.rasterization_info.lineWidth = 1.0f;

		info.multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		info.multisample_info.sampleShadingEnable = VK_FALSE;
		info.multisample_info.minSampleShading = 1.0f;
		info.multisample_info.pSampleMask = nullptr;
		info.multisample_info.alphaToCoverageEnable = VK_FALSE;
		info.multisample_info.alphaToOneEnable = VK_FALSE;

		info.color_blend_info.logicOpEnable = VK_FALSE;
		info.color_blend_info.logicOp = vk::LogicOp::eCopy;
		info.color_blend_info.attachmentCount = 0;
		info.color_blend_info.pAttachments = nullptr;
		info.color_blend_info.blendConstants[ 0 ] = 0.0f;
		info.color_blend_info.blendConstants[ 1 ] = 0.0f;
		info.color_blend_info.blendConstants[ 2 ] = 0.0f;
		info.color_blend_info.blendConstants[ 3 ] = 0.0f;

		info.depth_stencil_info.depthTestEnable = VK_TRUE;
		info.depth_stencil_info.depthWriteEnable = VK_TRUE;
		info.depth_stencil_info.depthCompareOp = vk::CompareOp::eLess;
		info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		info.depth_stencil_info.stencilTestEnable = VK_FALSE;
		//info.depth_stencil_info.front = {};
		//info.depth_stencil_info.back = {};
		info.depth_stencil_info.minDepthBounds = 0.0f;
		info.depth_stencil_info.maxDepthBounds = 1.0f;

		info.dynamic_state_enables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		info.dynamic_state_info.pDynamicStates = info.dynamic_state_enables.data();
		info.dynamic_state_info.dynamicStateCount = static_cast< std::uint32_t >( info.dynamic_state_enables.size() );
		//info.dynamic_state_info.flags = 0;

		info.binding_descriptions = Vertex::getBindingDescriptions();
		info.attribute_descriptions = Vertex::getAttributeDescriptions();
	}

	void PipelineConfigInfo::disableVertexInput( PipelineConfigInfo& info )
	{
		info.binding_descriptions = {};
		info.attribute_descriptions = {};
	}

	void PipelineConfigInfo::enableAlphaBlending( PipelineConfigInfo& info )
	{
		for ( std::size_t i = 0; i < info.color_blend_attachment.size(); ++i )
		{
			info.color_blend_attachment[ i ].blendEnable = VK_TRUE;
			info.color_blend_attachment[ i ].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
			info.color_blend_attachment[ i ].dstColorBlendFactor = vk::BlendFactor::eOne;
			info.color_blend_attachment[ i ].colorBlendOp = vk::BlendOp::eAdd;
			info.color_blend_attachment[ i ].srcAlphaBlendFactor = vk::BlendFactor::eOne;
			info.color_blend_attachment[ i ].dstAlphaBlendFactor = vk::BlendFactor::eZero;
			info.color_blend_attachment[ i ].alphaBlendOp = vk::BlendOp::eAdd;
			info.color_blend_attachment[ i ].colorWriteMask =
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA;
		}
	}

	void PipelineConfigInfo::addColorAttachmentConfig( PipelineConfigInfo& info )
	{
		vk::PipelineColorBlendAttachmentState state;
		state.blendEnable = VK_FALSE;
		state.srcColorBlendFactor = vk::BlendFactor::eOne;
		state.dstColorBlendFactor = vk::BlendFactor::eZero;
		state.colorBlendOp = vk::BlendOp::eAdd;
		state.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		state.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		state.alphaBlendOp = vk::BlendOp::eAdd;
		state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
		                     | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

		info.color_blend_attachment.emplace_back( state );

		info.color_blend_info.pAttachments = info.color_blend_attachment.data();
		info.color_blend_info.attachmentCount = info.color_blend_attachment.size();
	}

	PipelineConfigInfo::PipelineConfigInfo( vk::RenderPass pass )
	{
		render_pass = pass;
		defaultConfig( *this );
	}

} // namespace fgl::engine