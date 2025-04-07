//
// Created by kj16609 on 10/9/24.
//

#pragma once

#include "PipelineBuilder.hpp"
#include "engine/descriptors/DescriptorSet.hpp"

namespace fgl::engine

{
	namespace descriptors
	{
		class DescriptorSet;
	}

	class Pipeline
	{
		vk::raii::Pipeline m_pipeline;
		vk::raii::PipelineLayout m_layout;
		std::unique_ptr< PipelineBuilder::BuilderState > m_builder_state;
		vk::PipelineBindPoint m_bind_point;

		vk::raii::Pipeline rebuildPipeline();

	  public:

		Pipeline() = delete;
		Pipeline(
			vk::raii::Pipeline&& pipeline_in,
			vk::raii::PipelineLayout&& layout,
			vk::PipelineBindPoint bind_point,
			std::unique_ptr< PipelineBuilder::BuilderState >&& builder_state );

		void bind( CommandBuffer& cmd_buffer );

		void bindDescriptor(
			CommandBuffer&, descriptors::DescriptorIDX descriptor_idx, descriptors::DescriptorSet& set );
		void bindDescriptor( CommandBuffer& cmd_buffer, descriptors::DescriptorSet& set );

		void setDebugName( const char* str );

		template < typename T >
			requires std::is_trivially_copyable_v< T >
		void pushConstant( CommandBuffer& command_buffer, vk::ShaderStageFlags stage, const T& t )
		{
			command_buffer->pushConstants< T >( m_layout, stage, 0, { t } );
		}

		const vk::raii::PipelineLayout& layout() const { return m_layout; }
	};

} // namespace fgl::engine
