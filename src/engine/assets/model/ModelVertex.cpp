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
#include "ModelInstance.hpp"
#include "VertexAttribute.hpp"
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
			{ 1, sizeof( InstanceRenderInfo ), vk::VertexInputRate::eInstance }
		};

		return binding_descriptions;
	}

	std::vector< vk::VertexInputAttributeDescription > ModelVertex::getAttributeDescriptions()
	{
		AttributeBuilder builder { SimpleVertex::getAttributeDescriptions() };

#pragma GCC diagnostic push // TODO: Fix with reflection once we get it in 20 years
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
		// builder.add< decltype( ModelVertex::m_position ), offsetof( ModelVertex, m_position ) >( 0 );
		// builder.add< decltype( ModelVertex::m_color ), offsetof( ModelVertex, m_color ) >( 0 );
		builder.add< decltype( ModelVertex::m_normal ), offsetof( ModelVertex, m_normal ) >( 0 );
		builder.add< decltype( ModelVertex::m_tangent ), offsetof( ModelVertex, m_tangent ) >( 0 );
		builder.add< decltype( ModelVertex::m_uv ), offsetof( ModelVertex, m_uv ) >( 0 );
#pragma GCC diagnostic pop

		builder
			.add< decltype( InstanceRenderInfo::m_model_matrix ), offsetof( InstanceRenderInfo, m_model_matrix ) >( 1 );

		// builder.add<
		// 	decltype( InstanceRenderInfo::m_normal_matrix ),
		// 	offsetof( InstanceRenderInfo, m_normal_matrix ) >( 1 );

		builder
			.add< decltype( InstanceRenderInfo::m_material_id ), offsetof( InstanceRenderInfo, m_material_id ) >( 1 );

		return builder.get();
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
