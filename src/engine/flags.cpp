//
// Created by kj16609 on 10/18/24.
//

#include "flags.hpp"

#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::flags
{
	static bool should_reload_shaders { false };

	void triggerShaderReload()
	{
		log::debug( "Triggering shader reload" );
		should_reload_shaders = true;
	}

	bool shouldReloadShaders()
	{
		return should_reload_shaders;
	}

	void resetFlags()
	{
		should_reload_shaders = false;
	}

} // namespace fgl::engine::flags