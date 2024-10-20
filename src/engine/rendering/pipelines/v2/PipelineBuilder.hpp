//
// Created by kj16609 on 10/10/24.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <unordered_map>

#include "engine/rendering/pipelines/Shader.hpp"

namespace fgl::engine
{
	namespace descriptors
	{
		class DescriptorSetLayout;
		class DescriptorSetCollection;
	} // namespace descriptors
	class AttachmentBuilder;

	class Pipeline;

	class PipelineBuilder
	{
		using SetID = std::uint32_t;

		vk::raii::PipelineLayout createLayout();

	  public:

		void addDescriptorSet( SetID idx, const vk::raii::DescriptorSetLayout& descriptor_set_layout );
		void addDescriptorSet( descriptors::DescriptorSetLayout& descriptor );
		void addDynamicState( vk::DynamicState dynamic_state );
		void setPushConstant( vk::ShaderStageFlags flags, std::size_t size );

		PipelineBuilder() = delete;

		struct BuilderState
		{
			vk::raii::RenderPass& m_render_pass;
			std::uint32_t m_subpass_stage;
			vk::PushConstantRange push_constant {};

			std::vector< vk::PipelineShaderStageCreateInfo > m_stages {};
			std::vector< vk::DynamicState > m_dynamic_state {};

			struct
			{
				std::vector< vk::VertexInputBindingDescription > bindings {};
				std::vector< vk::VertexInputAttributeDescription > attributes {};
			} vertex_input_descriptions {};

			struct
			{
				std::shared_ptr< Shader > vertex { nullptr };
				std::shared_ptr< Shader > fragment { nullptr };
			} shaders {};

			std::unordered_map< SetID, vk::DescriptorSetLayout > descriptor_set_layouts {};

			vk::PipelineViewportStateCreateInfo viewport_info {};
			vk::PipelineInputAssemblyStateCreateInfo assembly_info {};
			vk::PipelineTessellationStateCreateInfo tesselation_state_info {};
			vk::PipelineTessellationDomainOriginStateCreateInfo tesselation_domain_info {};
			vk::PipelineRasterizationStateCreateInfo rasterization_info {};
			vk::PipelineMultisampleStateCreateInfo multisample_info {};

			std::vector< vk::PipelineColorBlendAttachmentState > color_blend_attachment {};
			vk::PipelineColorBlendAttachmentState& addColorAttachment();

			vk::PipelineColorBlendStateCreateInfo color_blend_info {};
			vk::PipelineDepthStencilStateCreateInfo depth_stencil_info {};

			std::vector< vk::DynamicState > dynamic_state_enables {};
			vk::PipelineDynamicStateCreateInfo dynamic_state_info {};

			vk::PipelineLayoutCreateInfo layout_info {};

			// Default config
			void setDefault();

			BuilderState( vk::raii::RenderPass& renderpass, const std::size_t subpass_stage ) :
			  m_render_pass( renderpass ),
			  m_subpass_stage( subpass_stage )
			{
				setDefault();
			}
		};

		std::unique_ptr< BuilderState > state;

		void setTopology( vk::PrimitiveTopology primitive_topology );
		void disableVertexInput();

		void disableCulling();

		[[nodiscard]] AttachmentBuilder addAttachment();
		[[nodiscard]] AttachmentBuilder addColorAttachment();

		void setBindingDescriptions( const std::vector< vk::VertexInputBindingDescription >& descriptions );

		void setAttributeDescriptions( const std::vector< vk::VertexInputAttributeDescription >& descriptions );

		PipelineBuilder( vk::raii::RenderPass& renderpass, std::uint32_t subpass_stage );

		void setVertexShader( std::shared_ptr< Shader >&& shader );

		void setFragmentShader( std::shared_ptr< Shader >&& shader );

		static vk::raii::Pipeline createFromState( BuilderState& state, vk::raii::PipelineLayout& layout );
		static vk::raii::Pipeline rebuildFromState( BuilderState& state, vk::raii::PipelineLayout& layout );

		std::unique_ptr< Pipeline > create();
	};

	//! Adds the GBuffer output attachments to the config for the given pipeline
	void setGBufferOutputAttachments( PipelineBuilder::BuilderState& config );

} // namespace fgl::engine
