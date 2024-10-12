//
// Created by kj16609 on 2/5/24.
//

#include "ModelVertex.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#pragma GCC diagnostic pop

#include "Model.hpp"
#include "engine/utils.hpp"

namespace fgl::engine
{

	bool ModelVertex::operator==( const ModelVertex& other ) const
	{
		return m_position == other.m_position && m_color == other.m_color && m_normal == other.m_normal
		    && m_uv == other.m_uv;
	}

	std::vector< vk::VertexInputBindingDescription > ModelVertex::getBindingDescriptions()
	{
		std::vector< vk::VertexInputBindingDescription > binding_descriptions {
			{ 0, sizeof( ModelVertex ), vk::VertexInputRate::eVertex },
			{ 1, sizeof( ModelMatrixInfo ), vk::VertexInputRate::eInstance }
		};

		return binding_descriptions;
	}

	std::vector< vk::VertexInputAttributeDescription > ModelVertex::getAttributeDescriptions()
	{
		std::vector< vk::VertexInputAttributeDescription > attribute_descriptions {
			SimpleVertex::getAttributeDescriptions()
		};

		// {location, binding, format, offset}
		// attribute_descriptions.emplace_back( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( ModelVertex, m_position ) );
		// attribute_descriptions.emplace_back( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( ModelVertex, m_color ) );
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
		attribute_descriptions.emplace_back( 2, 0, vk::Format::eR32G32B32Sfloat, offsetof( ModelVertex, m_normal ) );
		attribute_descriptions.emplace_back( 3, 0, vk::Format::eR32G32Sfloat, offsetof( ModelVertex, m_uv ) );
#pragma GCC diagnostic pop

		//Normal Matrix
		attribute_descriptions.emplace_back( 4, 1, vk::Format::eR32G32B32A32Sfloat, 0 );
		attribute_descriptions.emplace_back( 5, 1, vk::Format::eR32G32B32A32Sfloat, 1 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 6, 1, vk::Format::eR32G32B32A32Sfloat, 2 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 7, 1, vk::Format::eR32G32B32A32Sfloat, 3 * sizeof( glm::vec4 ) );

		attribute_descriptions.emplace_back( 8, 1, vk::Format::eR32Uint, offsetof( ModelMatrixInfo, albedo_id ) );
		attribute_descriptions.emplace_back( 9, 1, vk::Format::eR32Uint, offsetof( ModelMatrixInfo, normal_id ) );

		return attribute_descriptions;
	}

} // namespace fgl::engine

namespace std
{
	std::size_t hash< fgl::engine::ModelVertex >::operator()( const fgl::engine::ModelVertex& vertex ) const
	{
		std::size_t seed { 0 };
		fgl::engine::hashCombine( seed, vertex.m_position, vertex.m_color, vertex.m_normal, vertex.m_uv );
		return seed;
	}
} // namespace std
