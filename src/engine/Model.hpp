//
// Created by kj16609 on 11/28/23.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

#include "Device.hpp"

namespace fgl::engine
{

	class Model
	{
		Device& m_device;
		VkBuffer m_vertex_buffer;
		VkDeviceMemory m_buffer_memory;
		std::uint32_t m_vertex_count;

	  public:

		struct Vertex
		{
			glm::vec3 m_pos;
			glm::vec3 m_color;

			static std::vector< VkVertexInputBindingDescription > getBindingDescriptions();
			static std::vector< VkVertexInputAttributeDescription > getAttributeDescriptions();
		};

	  private:

		void createVertexBuffers( const std::vector< Vertex >& verts );

	  public:

		void bind( VkCommandBuffer buffer );
		void draw( VkCommandBuffer buffer );

		Model( Device& device, const std::vector< Vertex >& verts );

		~Model();

		Model( const Model& model ) = delete;
		Model& operator=( const Model& other ) = delete;
		Model( Model&& other ) = delete;
	};

} // namespace fgl::engine