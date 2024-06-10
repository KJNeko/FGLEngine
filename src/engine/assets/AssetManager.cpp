//
// Created by kj16609 on 6/6/24.
//

#include "AssetManager.hpp"

#include "stores.hpp"

namespace fgl::engine
{
	inline static TextureStore tex_store {};

	TextureStore& getTextureStore()
	{
		return tex_store;
	}

} // namespace fgl::engine
