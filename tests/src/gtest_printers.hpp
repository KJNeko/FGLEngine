//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include <catch2/catch_tostring.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace Catch
{
	template <>
	struct StringMaker< const glm::vec3 >
	{
		static std::string convert( glm::vec3 const & val ) { return glm::to_string( val ); }
	};

} // namespace Catch
