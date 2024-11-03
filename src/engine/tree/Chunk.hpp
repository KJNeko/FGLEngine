//
// Created by kj16609 on 11/2/24.
//

#pragma once
#include <memory>
#include <queue>
#include <unordered_map>

#include "engine/gameobjects/GameObject.hpp"
#include "glm/vec3.hpp"

namespace fgl::engine
{
	struct Frustum;
}

namespace fgl::engine::tree
{
	class Chunk;

	using ChunkID = std::size_t;

	class ChunkManager
	{
		std::unordered_map< ChunkID, std::shared_ptr< Chunk > > m_chunks {};
		std::queue< std::shared_ptr< Chunk > > m_delete_list {};

		std::shared_ptr< Chunk > createChunk( const ChunkID id );

		//! Deletes any chunks pending deletion
		void cleanup();

		//! Returns a shared pointer to the chunk with the given ID.
		std::shared_ptr< Chunk > getChunk( const ChunkID id );
	};

	ChunkID getID( const glm::vec3 point );
	glm::vec3 getPosition( const ChunkID id );

	class Chunk
	{
		//! Determines if the chunk is active to the rendering system
		bool m_rendering_active { true };

		const ChunkID m_id;
		const glm::vec3 m_center;

		//! Contains a list of all objects within this chunk
		std::unordered_map< GameObject::GameObjectID, std::shared_ptr< GameObject > > m_objects {};

	  public:

		Chunk() = delete;
		Chunk( const ChunkID id );

		void addGameObject( std::shared_ptr< GameObject > object );

		// Size of a chunk from center to
		constexpr static float CHUNK_SIZE { 100.0f };
		constexpr static float CHUNK_HALF { CHUNK_SIZE / 2.0f };

		//! Returns the number of game objects within this chunk
		std::size_t childCount();

		//! Returns true if the bounds of this chunk are visible.
		bool isVisible( const Frustum& frustum ) const;
	};

} // namespace fgl::engine::tree
