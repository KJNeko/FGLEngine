//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include <filesystem>
#include <vector>

#include "PipelineConfigInfo.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{
	struct ShaderHandle;
}

namespace fgl::engine::internal
{

	class Pipeline
	{
	  protected:

		Device& m_device;
		vk::Pipeline m_vk_pipeline { VK_NULL_HANDLE };
		vk::PipelineLayout m_layout { VK_NULL_HANDLE };
		vk::ShaderModule m_vert_shader { VK_NULL_HANDLE };
		vk::ShaderModule m_frag_shader { VK_NULL_HANDLE };

		void createGraphicsPipeline(
			std::vector< std::unique_ptr< ShaderHandle > >& shaders, const PipelineConfigInfo& info );

	  public:

		Pipeline( Device& device ) : m_device( device ) {}

		~Pipeline();

		Pipeline( const Pipeline& other ) = delete;
		Pipeline& operator=( const Pipeline& ) = delete;

		void bind( vk::CommandBuffer command_buffer );
	};
} // namespace fgl::engine::internal