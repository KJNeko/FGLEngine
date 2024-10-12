//
// Created by kj16609 on 8/5/24.
//

#pragma once
#include <string_view>
#include <vector>

namespace shaderc
{
	class Compiler;
}

namespace fgl::engine
{

	shaderc::Compiler& getInstance();

	std::vector< std::byte > compileShader( const std::string_view input_name, const std::vector< std::byte >& input );

} // namespace fgl::engine