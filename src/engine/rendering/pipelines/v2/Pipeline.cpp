//
// Created by kj16609 on 10/9/24.
//

#include "Pipeline.hpp"

#include "PipelineBuilder.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/flags.hpp"

namespace fgl::engine
{

	vk::raii::Pipeline Pipeline::rebuildPipeline()
	{
		return PipelineBuilder::rebuildFromState( *m_builder_state, m_layout );
	}

	Pipeline::Pipeline(
		vk::raii::Pipeline&& pipeline_in,
		vk::raii::PipelineLayout&& layout,
		std::unique_ptr< PipelineBuilder::BuilderState >&& builder_state ) :
	  m_pipeline( std::move( pipeline_in ) ),
	  m_layout( std::move( layout ) ),
	  m_builder_state( std::forward< std::unique_ptr< PipelineBuilder::BuilderState > >( builder_state ) )
	{}

	void Pipeline::bind( vk::raii::CommandBuffer& cmd_buffer )
	{
		if ( flags::shouldReloadShaders() )
		{
			m_pipeline = rebuildPipeline();
		}

		cmd_buffer.bindPipeline( vk::PipelineBindPoint::eGraphics, m_pipeline );
	}

	void Pipeline::bindDescriptor(
		vk::raii::CommandBuffer& command_buffer,
		const descriptors::DescriptorIDX descriptor_idx,
		descriptors::DescriptorSet& set )
	{
		const std::vector< vk::DescriptorSet > sets { *set };
		constexpr std::vector< std::uint32_t > offsets {};

		command_buffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, m_layout, descriptor_idx, sets, offsets );
	}

	void Pipeline::bindDescriptor( vk::raii::CommandBuffer& comd_buffer, descriptors::DescriptorSet& set )
	{
		bindDescriptor( comd_buffer, set.setIDX(), set );
	}

	void Pipeline::setDebugName( const char* str )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::ePipeline;
		info.pObjectName = str;
		info.objectHandle = reinterpret_cast< std::uint64_t >( static_cast< VkPipeline >( *this->m_pipeline ) );
		Device::getInstance().setDebugUtilsObjectName( info );
	}

} // namespace fgl::engine