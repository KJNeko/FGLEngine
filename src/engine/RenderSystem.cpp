//
// Created by kj16609 on 11/27/23.
//

#include "RenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ON
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "Camera.hpp"
#include "engine/debug_gui/DebugGUI.hpp"

namespace fgl::engine
{
	struct SimplePushConstantData
	{
		glm::mat4 transform { 1.0f };
		alignas( 16 ) glm::vec3 color;
	};

	void RenderSystem::createPipelineLayout()
	{
		VkPushConstantRange push_constant_range { .stageFlags =
			                                          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			                                      .offset = 0,
			                                      .size = sizeof( SimplePushConstantData ) };

		VkPipelineLayoutCreateInfo pipeline_layout_info { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			                                              .setLayoutCount = 0,
			                                              .pSetLayouts = nullptr,
			                                              .pushConstantRangeCount = 1,
			                                              .pPushConstantRanges = &push_constant_range };

		if ( vkCreatePipelineLayout( m_device.device(), &pipeline_layout_info, nullptr, &m_pipeline_layout )
		     != VK_SUCCESS )
			throw std::runtime_error( "Failed to create pipeline layout" );
	}

	void RenderSystem::createPipeline( VkRenderPass render_pass )
	{
		PipelineConfigInfo pipeline_config {};
		Pipeline::defaultConfig( pipeline_config );

		pipeline_config.render_pass = render_pass;
		pipeline_config.layout = m_pipeline_layout;

		m_pipeline = std::make_unique<
			Pipeline >( m_device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipeline_config );
	}

	RenderSystem::RenderSystem( Device& device, VkRenderPass render_pass ) : m_device( device )
	{
		createPipelineLayout();
		createPipeline( render_pass );
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout( m_device.device(), m_pipeline_layout, nullptr );
	}

	void RenderSystem::renderGameObjects(
		VkCommandBuffer command_buffer, std::vector< GameObject >& game_objects, const Camera& camera )
	{
		//TracyVkZone( m_device.getCurrentTracyCTX(), command_buffer, "Render game objects" );
		m_pipeline->bind( command_buffer );

		for ( auto& obj : game_objects )
		{
			obj.transform.rotation.y = glm::mod( obj.transform.rotation.y + 0.01f, glm::two_pi< float >() );
			obj.transform.rotation.x = glm::mod( obj.transform.rotation.x + 0.01f, glm::two_pi< float >() );
			obj.transform.rotation.z = glm::mod( obj.transform.rotation.z + 0.01f, glm::two_pi< float >() );

			SimplePushConstantData push { .transform = camera.getProjectionMatrix() * obj.transform.mat4(),
				                          .color = obj.color };

			vkCmdPushConstants(
				command_buffer,
				m_pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof( SimplePushConstantData ),
				&push );

			obj.model->bind( command_buffer );
			obj.model->draw( command_buffer );
		}
	}

} // namespace fgl::engine
