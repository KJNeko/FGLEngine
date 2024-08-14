//
// Created by kj16609 on 8/12/24.
//

#pragma once
#include <glm/vec3.hpp>

#include <vector>

namespace vk
{
	struct VertexInputAttributeDescription;
	struct VertexInputBindingDescription;
}

namespace fgl::engine
{
	struct SimpleVertex
	{
		glm::vec3 m_position { 0.0f };
		glm::vec3 m_color { 1.0f };

		SimpleVertex() = default;

		SimpleVertex( const glm::vec3 pos, const glm::vec3 color ) : m_position( pos ), m_color( color ) {}

		static std::vector< vk::VertexInputBindingDescription > getBindingDescriptions();
		static std::vector< vk::VertexInputAttributeDescription > getAttributeDescriptions();
	};


}
