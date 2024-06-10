//
// Created by kj16609 on 6/7/24.
//

#pragma once

#include "engine/texture/Texture.hpp"

namespace fgl::engine
{
	class Texture;

	using TextureStore = AssetStore< Texture>;

	TextureStore& getTextureStore();

} // namespace fgl::engine