//
// Created by kj16609 on 2/5/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

#include <vulkan/vulkan.hpp>

#include "SimpleVertex.hpp"

namespace fgl::engine
{

	struct ModelVertex : public SimpleVertex
	{
		glm::vec3 m_normal { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_tangent { 0.0f, 0.0f, 0.0f };
		glm::vec2 m_uv { 0.0f, 0.0f };

		ModelVertex( const glm::vec3 pos, const glm::vec3 color, const glm::vec3 norm, const glm::vec2 uv ) noexcept :
		  SimpleVertex( pos, color ),
		  m_normal( norm ),
		  m_uv( uv )
		{}

		static std::vector< vk::VertexInputBindingDescription > getBindingDescriptions();
		static std::vector< vk::VertexInputAttributeDescription > getAttributeDescriptions();

		ModelVertex() noexcept = default;

		bool operator==( const ModelVertex& other ) const;
	};

} // namespace fgl::engine

namespace std
{

	template <>
	struct hash< fgl::engine::ModelVertex >
	{
		std::size_t operator()( const fgl::engine::ModelVertex& vertex ) const;
	};

} // namespace std
