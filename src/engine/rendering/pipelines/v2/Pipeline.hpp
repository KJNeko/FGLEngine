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

		vk::raii::Pipeline rebuildPipeline();

	  public:

		Pipeline() = delete;
		Pipeline(
			vk::raii::Pipeline&& pipeline_in,
			vk::raii::PipelineLayout&& layout,
			std::unique_ptr< PipelineBuilder::BuilderState >&& builder_state );

		void bind( vk::raii::CommandBuffer& );

		void bindDescriptor(
			vk::raii::CommandBuffer&, descriptors::DescriptorIDX descriptor_idx, descriptors::DescriptorSet& set );
		void bindDescriptor( vk::raii::CommandBuffer& comd_buffer, descriptors::DescriptorSet& set );

		void setDebugName( const char* str );

		template < typename T >
			requires std::is_trivially_copyable_v< T >
		void pushConstant( vk::raii::CommandBuffer& command_buffer, vk::ShaderStageFlags stage, const T& t )
		{
			command_buffer.pushConstants< T >( m_layout, stage, 0, { t } );
		}
	};

} // namespace fgl::engine
