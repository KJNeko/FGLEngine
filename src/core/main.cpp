//
// Created by kj16609 on 11/27/23.
//

#include <iostream>

#include "engine/EngineContext.hpp"

int main()
try
{
	fgl::engine::EngineContext engine_ctx {};

	engine_ctx.run();

	return EXIT_SUCCESS;
}
catch ( const std::exception& e )
{
	std::cerr << "\n\n Exception caught:\n\t" << e.what() << std::endl;
	std::abort();
}
catch ( ... )
{
	std::cerr << "\n\n An unknown error has occured.\n";
	std::abort();
}
