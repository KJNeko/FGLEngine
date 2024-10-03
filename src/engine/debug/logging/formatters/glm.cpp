//
// Created by kj16609 on 10/3/24.
//

#include "glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

std::format_context::iterator std::formatter< glm::qua< float > >::format( const glm::quat& quat, format_context& ctx )
	const
{
	return format_to( ctx.out(), "quat( w: {:10f}, x: {:10f}, y: {:10f}, z: {:10f} )", quat.w, quat.x, quat.y, quat.z );
}