//
// Created by kj16609 on 8/5/24.
//

#pragma once
#include <filesystem>
#include <vector>

namespace fgl::engine
{
	enum ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

	std::vector< std::byte > compileShader( const std::filesystem::path& input_name, ShaderType type );

} // namespace fgl::engine