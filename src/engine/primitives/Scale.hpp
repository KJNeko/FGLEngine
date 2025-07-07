//
// Created by kj16609 on 2/29/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

#include <cmath>

namespace fgl::engine
{

	struct Scale : public glm::vec3
	{
		using glm::vec3::x;
		using glm::vec3::y;
		using glm::vec3::z;

		Scale( const glm::vec3 val ) : glm::vec3( val )
		{
			assert( !std::isnan( x ) );
			assert( !std::isnan( y ) );
			assert( !std::isnan( z ) );
		}

		Scale( const float val ) : glm::vec3( val ) {}

		Scale( const float x_i, const float y_i, const float z_i ) : glm::vec3( x_i, y_i, z_i ) {}
	};

} // namespace fgl::engine
