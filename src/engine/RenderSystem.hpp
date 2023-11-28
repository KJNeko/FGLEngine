//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>
#include <vector>

#include "Camera.hpp"
#include "Device.hpp"
#include "GameObject.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"

namespace fgl::engine
{

	class RenderSystem
	{
		Device& m_device;

		std::unique_ptr< Pipeline > m_pipeline;
		VkPipelineLayout m_pipeline_layout;

		void createPipelineLayout();
		void createPipeline( VkRenderPass render_pass );

	  public:

		void renderGameObjects(
			VkCommandBuffer command_buffer, std::vector< GameObject >& objects, const Camera& camera );

		RenderSystem( Device& device, VkRenderPass render_pass );
		~RenderSystem();
		RenderSystem( RenderSystem&& other ) = delete;
		RenderSystem( const RenderSystem& other ) = delete;
		RenderSystem& operator=( const RenderSystem& other ) = delete;
	};

} // namespace fgl::engine
