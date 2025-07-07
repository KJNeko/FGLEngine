//
// Created by kj16609 on 6/6/24.
//

#include "AssetManager.hpp"

#include "stores.hpp"

namespace fgl::engine
{
	inline static TextureStore TEX_STORE {};

	TextureStore& getTextureStore()
	{
		return TEX_STORE;
	}

} // namespace fgl::engine
