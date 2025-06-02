//
// Created by kj16609 on 6/1/25.
//

#define TEST_PUBLIC public:
#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "context/EngineContext.hpp"
#include "context/Window.hpp"

using namespace fgl::engine;

TEST_CASE( "Engine Context", "[context][engine][init]" )
{
	WHEN( "Engine Context is initalized" )
	{
		THEN( "No exception should be thrown" )
		{
			EngineContext ctx {};

			AND_THEN( "The window instance should be created" )
			{
				// Check that the window was constructed
				REQUIRE( ( ctx.m_window->m_window != nullptr ) );

				// Just so I can see the window
				// std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
			}
		}
	}
}
