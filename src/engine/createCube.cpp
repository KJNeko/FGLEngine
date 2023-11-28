// temporary helper function, creates a 1x1x1 cube centered at offset

#include <memory>

#include "Model.hpp"

using namespace fgl::engine;

std::unique_ptr< Model > createCubeModel( Device& device, glm::vec3 offset )
{
	std::vector< Model::Vertex > vertices {

		// left face (white)
		{ { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
		{ { -.5f, .5f, .5f }, { .9f, .9f, .9f } },
		{ { -.5f, -.5f, .5f }, { .9f, .9f, .9f } },
		{ { -.5f, -.5f, -.5f }, { .9f, .9f, .9f } },
		{ { -.5f, .5f, -.5f }, { .9f, .9f, .9f } },
		{ { -.5f, .5f, .5f }, { .9f, .9f, .9f } },

		// right face (yellow)
		{ { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
		{ { .5f, .5f, .5f }, { .8f, .8f, .1f } },
		{ { .5f, -.5f, .5f }, { .8f, .8f, .1f } },
		{ { .5f, -.5f, -.5f }, { .8f, .8f, .1f } },
		{ { .5f, .5f, -.5f }, { .8f, .8f, .1f } },
		{ { .5f, .5f, .5f }, { .8f, .8f, .1f } },

		// top face (orange, remember y axis points down)
		{ { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
		{ { .5f, -.5f, .5f }, { .9f, .6f, .1f } },
		{ { -.5f, -.5f, .5f }, { .9f, .6f, .1f } },
		{ { -.5f, -.5f, -.5f }, { .9f, .6f, .1f } },
		{ { .5f, -.5f, -.5f }, { .9f, .6f, .1f } },
		{ { .5f, -.5f, .5f }, { .9f, .6f, .1f } },

		// bottom face (red)
		{ { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
		{ { .5f, .5f, .5f }, { .8f, .1f, .1f } },
		{ { -.5f, .5f, .5f }, { .8f, .1f, .1f } },
		{ { -.5f, .5f, -.5f }, { .8f, .1f, .1f } },
		{ { .5f, .5f, -.5f }, { .8f, .1f, .1f } },
		{ { .5f, .5f, .5f }, { .8f, .1f, .1f } },

		// nose face (blue)
		{ { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
		{ { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },
		{ { -.5f, .5f, 0.5f }, { .1f, .1f, .8f } },
		{ { -.5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
		{ { .5f, -.5f, 0.5f }, { .1f, .1f, .8f } },
		{ { .5f, .5f, 0.5f }, { .1f, .1f, .8f } },

		// tail face (green)
		{ { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
		{ { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },
		{ { -.5f, .5f, -0.5f }, { .1f, .8f, .1f } },
		{ { -.5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
		{ { .5f, -.5f, -0.5f }, { .1f, .8f, .1f } },
		{ { .5f, .5f, -0.5f }, { .1f, .8f, .1f } },

	};
	for ( auto& v : vertices )
	{
		v.m_pos += offset;
	}
	return std::make_unique< Model >( device, vertices );
}