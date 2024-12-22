//
// Created by kj16609 on 10/10/24.
//

#include "AttachmentBuilder.hpp"

#include "PipelineBuilder.hpp"

namespace fgl::engine
{

	AttachmentBuilder::AttachmentBuilder( PipelineBuilder& source ) : parent( source )
	{
		// Default
		color_blend_config.blendEnable = VK_FALSE;
		color_blend_config.srcColorBlendFactor = vk::BlendFactor::eOne;
		color_blend_config.dstColorBlendFactor = vk::BlendFactor::eZero;
		color_blend_config.colorBlendOp = vk::BlendOp::eAdd;
		color_blend_config.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		color_blend_config.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		color_blend_config.alphaBlendOp = vk::BlendOp::eAdd;
		color_blend_config.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
		                                  | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	}

	AttachmentBuilder& AttachmentBuilder::enableBlend()
	{
		color_blend_config.blendEnable = VK_TRUE;
		return *this;
	}

	void AttachmentBuilder::finish()
	{
		parent.m_state->color_blend_attachment.emplace_back( color_blend_config );
		parent.m_state->formats.colors.emplace_back( m_format );
		m_finished = true;
	}

	AttachmentBuilder::~AttachmentBuilder()
	{
		FGL_ASSERT( m_finished, "Attachemnt builder not finished!" );
	}

	AttachmentBuilder& AttachmentBuilder::setFormat( const vk::Format format )
	{
		m_format = format;
		return *this;
	}
} // namespace fgl::engine
