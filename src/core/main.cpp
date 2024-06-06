//
// Created by kj16609 on 11/27/23.
//

#include <iostream>

#include "engine/EngineContext.hpp"
#include "engine/logging/logging.hpp"

int main()
{
	fgl::engine::log::set_level( spdlog::level::debug );

	fgl::engine::EngineContext engine_ctx {};

	engine_ctx.run();

	return EXIT_SUCCESS;
}
