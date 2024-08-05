//
// Created by kj16609 on 8/5/24.
//

#pragma once

#include <shaderc/shaderc.hpp>

namespace fgl::engine
{

	shaderc::Compiler& getInstance();

	std::vector< std::byte > compileShader( const std::string_view str, const std::vector< std::byte >& input );

} // namespace fgl::engine