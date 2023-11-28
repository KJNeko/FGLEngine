//
// Created by kj16609 on 11/28/23.
//

#include "Camera.hpp"

#include <cassert>
#include <limits>

namespace fgl::engine
{

	void Camera::setOrthographicProjection( float left, float right, float top, float bottom, float near, float far )
	{
		projection_matrix = glm::mat4 { 1.0f };
		projection_matrix[ 0 ][ 0 ] = 2.0f / ( right - left );
		projection_matrix[ 1 ][ 1 ] = 2.0f / ( bottom - top );
		projection_matrix[ 2 ][ 2 ] = 1.0f / ( far - near );
		projection_matrix[ 3 ][ 0 ] = -( right + left ) / ( right - left );
		projection_matrix[ 3 ][ 1 ] = -( bottom + top ) / ( bottom - top );
		projection_matrix[ 3 ][ 2 ] = -near / ( far - near );
	}

	void Camera::setPerspectiveProjection( float fovy, float aspect, float near, float far )
	{
		assert( glm::abs( aspect - std::numeric_limits< float >::epsilon() ) > 0 );
		const float tan_half_fovy { std::tan( fovy / 2.0f ) };
		projection_matrix = glm::mat4 { 0.0f };
		projection_matrix[ 0 ][ 0 ] = 1.0f / ( aspect * tan_half_fovy );
		projection_matrix[ 1 ][ 1 ] = 1.0f / ( tan_half_fovy );
		projection_matrix[ 2 ][ 2 ] = far / ( far - near );
		projection_matrix[ 2 ][ 3 ] = 1.0f;
		projection_matrix[ 3 ][ 2 ] = -( far * near ) / ( far - near );
	}

} // namespace fgl::engine