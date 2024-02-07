//
// Created by kj16609 on 2/5/24.
//

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>

namespace fgl::engine
{

	struct Vertex
	{
		glm::vec3 m_position { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_color { 1.0f, 1.0f, 1.0f };
		glm::vec3 m_normal { 0.0f, 0.0f, 0.0f };
		glm::vec2 m_uv { 0.0f, 0.0f };

		static std::vector< vk::VertexInputBindingDescription > getBindingDescriptions();
		static std::vector< vk::VertexInputAttributeDescription > getAttributeDescriptions();

		Vertex() noexcept = default;

		bool operator==( const Vertex& other ) const;
	};

} // namespace fgl::engine

namespace std
{

	template <>
	struct hash< fgl::engine::Vertex >
	{
		std::size_t operator()( const fgl::engine::Vertex& vertex ) const;
	};

} // namespace std
