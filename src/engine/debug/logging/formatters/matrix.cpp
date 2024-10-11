//
// Created by kj16609 on 10/1/24.
//

#include "matrix.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

std::format_context::iterator std::formatter< glm::vec4 >::format( const glm::vec4& vec, format_context& ctx ) const
{
	return format_to( ctx.out(), "vec4( x: {:10f}, y: {:10f}, z: {:10f}, w: {:10f} )", vec.x, vec.y, vec.z, vec.w );
}

std::format_context::iterator std::formatter< glm::vec3 >::format( const glm::vec3& vec, format_context& ctx ) const
{
	return format_to( ctx.out(), "vec3( x: {}, y: {}, z: {} )", vec.x, vec.y, vec.z );
}

std::format_context::iterator std::formatter< glm::mat4 >::format( const glm::mat4& mat, format_context& ctx ) const
{
	return format_to( ctx.out(), "mat4: [\n{},\n{},\n{},\n{} ]", mat[ 0 ], mat[ 1 ], mat[ 2 ], mat[ 3 ] );
}