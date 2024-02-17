//
// Created by kj16609 on 2/15/24.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/primitives/Vector.hpp"

namespace Catch
{
	template <>
	struct StringMaker< glm::vec3 >
	{
		static std::string convert( const glm::vec3& vec ) { return glm::to_string( vec ); }
	};

	template <>
	struct StringMaker< fgl::engine::Vector >
	{
		static std::string convert( const fgl::engine::Vector& vec )
		{
			return StringMaker< glm::vec3 >::convert( static_cast< glm::vec3 >( vec ) );
		}
	};

} // namespace Catch

namespace glm
{
	inline bool operator==( const glm::vec3& lhs, const glm::vec3& rhs )
	{
		return glm::all( ::glm::epsilonEqual( lhs, rhs, std::numeric_limits< float >::epsilon() ) );
	}

} // namespace glm
