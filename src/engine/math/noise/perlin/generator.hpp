//
// Created by kj16609 on 3/27/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/vec2.hpp>
#pragma GCC diagnostic pop

#include <vector>

namespace fgl::engine
{

	float perlin( glm::vec2 coord, std::size_t seed = 0 );
	[[nodiscard]] std::vector< std::byte >
		generatePerlinImage( const glm::vec< 2, std::size_t > size, const int octives, const std::size_t seed = 0 );

} // namespace fgl::engine
