//
// Created by kj16609 on 6/2/25.
//
#pragma once
#include <format>
#include <iostream>

namespace fgl::engine::log
{

	template < typename... Ts >
	void verbose( std::format_string< Ts... > str, Ts&&... args )
	{
		std::cout << "[VERBOSE]: << " << std::format< Ts... >( str, std::forward< Ts >( args )... ) << std::endl;
	}

} // namespace fgl::engine::log
