//
// Created by kj16609 on 3/16/24.
//

#include "terrainModel.hpp"

#include "engine/model/Model.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{

	std::shared_ptr< Model > generateTerrainModel( memory::Buffer& vertex_buffer, memory::Buffer& index_buffer )
	{
		std::vector< Vertex > verts {};

		constexpr glm::vec3 TOP_LEFT { -0.5f, 0.5f, 0.0f };
		constexpr glm::vec3 TOP_RIGHT { 0.5f, 0.5f, 0.0f };
		constexpr glm::vec3 BOTTOM_RIGHT { 0.5f, -0.5f, 0.0f };
		constexpr glm::vec3 BOTTOM_LEFT { -0.5f, -0.5f, 0.0f };
		constexpr float dist_mulpt { 2.0f };

		verts.emplace_back( TOP_LEFT * dist_mulpt, glm::vec3( 1.0f ), constants::WORLD_UP, glm::vec2( 0.0f, 0.0f ) );
		verts.emplace_back( TOP_RIGHT * dist_mulpt, glm::vec3( 1.0f ), constants::WORLD_UP, glm::vec2( 1.0f, 0.0f ) );
		verts
			.emplace_back( BOTTOM_RIGHT * dist_mulpt, glm::vec3( 1.0f ), constants::WORLD_UP, glm::vec2( 1.0f, 1.0f ) );
		verts.emplace_back( BOTTOM_LEFT * dist_mulpt, glm::vec3( 1.0f ), constants::WORLD_UP, glm::vec2( 0.0f, 1.0f ) );

		std::vector< std::uint32_t > indicies { 0, 1, 2, 3 };

		std::shared_ptr< Model > model {
			Model::createModelFromVerts( std::move( verts ), std::move( indicies ), vertex_buffer, index_buffer )
		};

		return model;
	}

} // namespace fgl::engine
