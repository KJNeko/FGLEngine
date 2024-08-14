//
// Created by kj16609 on 3/14/24.
//

#pragma once
#include "engine/gameobjects/GameObject.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/texture/Texture.hpp"
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

	inline bool defaultTrueFunc( [[maybe_unused]] const GameObject& )
	{
		return true;
	}

	enum TreeFilterFlags
	{
		IS_TEXTURELESS = 1 << 0,
		DEFAULT_FLAGS = 0,
	};

	std::pair< std::vector< vk::DrawIndexedIndirectCommand >, std::vector< ModelMatrixInfo > > getDrawCallsFromTree(
		OctTreeNode& root,
		const Frustum& frustum,
		GameObjectFlagType game_object_flags,
		TreeFilterFlags tree_flags = DEFAULT_FLAGS,
		std::function< bool( const GameObject& ) > filterFunc = &defaultTrueFunc );

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
