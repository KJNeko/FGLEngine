//
// Created by kj16609 on 8/12/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

#include <vector>

namespace vk
{
	struct VertexInputAttributeDescription;
	struct VertexInputBindingDescription;
} // namespace vk

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

} // namespace fgl::engine
