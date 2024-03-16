//
// Created by kj16609 on 3/14/24.
//

#pragma once
#include "engine/GameObject.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/texture/TextureHandle.hpp"
#include "engine/utils.hpp"

namespace fgl::engine
{
	class OctTreeNode;
	struct ModelMatrixInfo;
	// <TextureID, MemoryOffset>
	using DrawKey = std::pair< TextureID, vk::DeviceSize >;

	using DrawPair = std::pair< vk::DrawIndexedIndirectCommand, std::vector< ModelMatrixInfo > >;

	inline bool operator<( const DrawPair& left, const DrawPair& right )
	{
		return left.first.firstIndex < right.first.firstIndex;
	}

	inline bool operator==( const DrawPair& left, const DrawPair& right )
	{
		return left.first.firstIndex == right.first.firstIndex && left.first.indexCount && right.first.indexCount;
	}

	std::pair< std::vector< vk::DrawIndexedIndirectCommand >, std::vector< ModelMatrixInfo > > getDrawCallsFromTree(
		OctTreeNode& root, const Frustum< CoordinateSpace::World >& frustum, const GameObjectFlagType flags );

} // namespace fgl::engine

namespace std
{
	template <>
	struct hash< fgl::engine::DrawKey >
	{
		inline size_t operator()( const fgl::engine::DrawKey& key ) const
		{
			const auto id_hash { std::hash< fgl::engine::TextureID >()( key.first ) };
			const auto offset_hash { std::hash< vk::DeviceSize >()( key.second ) };

			size_t seed { 0 };
			fgl::engine::hashCombine( seed, id_hash, offset_hash );
			return seed;
		}
	};

} // namespace std
