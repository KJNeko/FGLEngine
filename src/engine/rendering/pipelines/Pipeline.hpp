//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include <filesystem>
#include <vector>

#include "PipelineConfigInfo.hpp"

namespace fgl::engine
{
	class Device;
	struct ShaderHandle;
}

namespace fgl::engine::internal
{

	class Pipeline
	{
	  protected:

		Device& m_device;
		vk::raii::PipelineLayout m_layout;
		vk::raii::Pipeline m_vk_pipeline;
		vk::ShaderModule m_vert_shader { VK_NULL_HANDLE };
		vk::ShaderModule m_frag_shader { VK_NULL_HANDLE };

		vk::raii::Pipeline createGraphicsPipeline(
			std::vector< std::unique_ptr< ShaderHandle > >& shaders,
			const PipelineConfigInfo& info,
			const vk::raii::PipelineLayout& layout );

	  public:

		Pipeline(
			Device& device,
			vk::raii::PipelineLayout layout,
			PipelineConfigInfo info,
			std::vector< std::unique_ptr< ShaderHandle > > shaders );

		Pipeline( const Pipeline& other ) = delete;
		Pipeline& operator=( const Pipeline& ) = delete;

		void bind( vk::raii::CommandBuffer& command_buffer );

		void setDebugName( const std::string str );
	};
} // namespace fgl::engine::internal