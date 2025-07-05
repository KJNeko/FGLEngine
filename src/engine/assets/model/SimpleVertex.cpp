//
// Created by kj16609 on 8/12/24.
//

#include "SimpleVertex.hpp"

#include <vector>

#include "ModelVertex.hpp"
#include "VertexAttribute.hpp"

namespace fgl::engine
{

	std::vector< vk::VertexInputBindingDescription > SimpleVertex::getBindingDescriptions()
	{
		// {buffer_idx, stride, rate}
		return { { 0, sizeof( SimpleVertex ), vk::VertexInputRate::eVertex } };
	}

	std::vector< vk::VertexInputAttributeDescription > SimpleVertex::getAttributeDescriptions()
	{
		AttributeBuilder builder {};

		builder.add< decltype( SimpleVertex::m_position ), offsetof( SimpleVertex, m_position ) >( 0 );
		builder.add< decltype( SimpleVertex::m_color ), offsetof( SimpleVertex, m_color ) >( 0 );

		return builder.get();
	}

} // namespace fgl::engine
