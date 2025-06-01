//
// Created by kj16609 on 8/12/24.
//

#include "SimpleVertex.hpp"

#include <vector>

#include "ModelVertex.hpp"

namespace fgl::engine
{

	std::vector< vk::VertexInputBindingDescription > SimpleVertex::getBindingDescriptions()
	{
		// {buffer_idx, stride, rate}
		constexpr auto stride = sizeof( SimpleVertex );
		std::vector< vk::VertexInputBindingDescription > binding_descriptions {
			{ 0, stride, vk::VertexInputRate::eVertex }
		};

		return binding_descriptions;
	}

	std::vector< vk::VertexInputAttributeDescription > SimpleVertex::getAttributeDescriptions()
	{
		// {location, binding, format, offset}
		return {
			{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( SimpleVertex, m_position ) },
			{ 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( SimpleVertex, m_color ) },
		};
	}

} // namespace fgl::engine
