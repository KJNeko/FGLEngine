//
// Created by kj16609 on 2/29/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

namespace fgl::engine
{

	struct Scale : public glm::vec3
	{
		using glm::vec3::x;
		using glm::vec3::y;
		using glm::vec3::z;

		Scale( const glm::vec3 value ) : glm::vec3( value ) {}

		Scale( const float value ) : glm::vec3( value ) {}

		Scale( const float x_i, const float y_i, const float z_i ) : glm::vec3( x_i, y_i, z_i ) {}
	};

} // namespace fgl::engine
