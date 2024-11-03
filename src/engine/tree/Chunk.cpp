//
// Created by kj16609 on 11/3/24.
//

#include "Chunk.hpp"

#include "engine/utils.hpp"

namespace fgl::engine::tree
{

	std::shared_ptr< Chunk > ChunkManager::createChunk( const ChunkID id )
	{
		std::shared_ptr< Chunk > chunk { std::make_shared< Chunk >( id ) };

		m_chunks.insert( std::make_pair( id, chunk ) );

		return chunk;
	}

	std::shared_ptr< Chunk > ChunkManager::getChunk( const ChunkID id )
	{}

	ChunkID getID( const glm::vec3 point )
	{
		// the inital chunk starts at 0,0,0. meaning that the bounds of the inital chunk goes from
		// +/- CHUNK_SIZE / 2.0f

		// We need to start by dividing the point into the size of the chunks.
		constexpr glm::vec3 CHUNK_VEC3 { Chunk::CHUNK_HALF };
		const glm::vec3 chunk_pos { point / CHUNK_VEC3 };

		// chunk_pos now needs to be clamped to interegers, we always round DOWN.
		const glm::vec3 round_offset { glm::sign( chunk_pos ) * glm::vec3( 0.5f ) };
		const glm::vec3 chunk_coords { glm::round( chunk_pos + round_offset ) };
		const glm::vec< 3, int > chunk_coords_i { chunk_coords };

		ChunkID hash { 0 };
		engine::hashCombine( hash, chunk_coords_i[ 0 ], chunk_coords_i[ 1 ], chunk_coords_i[ 2 ] );

		return hash;
	}

	Chunk::Chunk( const ChunkID id ) : m_id( id ), m_center( getPosition( id ) )
	{}

} // namespace fgl::engine::tree