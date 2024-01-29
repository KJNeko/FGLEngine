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
		ZoneScoped;
		projection_matrix = glm::mat4 { 1.0f };
		projection_matrix[ 0 ][ 0 ] = 2.0f / ( right - left );
		projection_matrix[ 1 ][ 1 ] = 2.0f / ( bottom - top );
		projection_matrix[ 2 ][ 2 ] = 1.0f / ( far - near );
		projection_matrix[ 3 ][ 0 ] = -( right + left ) / ( right - left );
		projection_matrix[ 3 ][ 1 ] = -( bottom + top ) / ( bottom - top );
		projection_matrix[ 3 ][ 2 ] = -near / ( far - near );

		//TODO: Figure out frustum culling for orthographic projection. (If we even wanna use it)
	}

	void Camera::setPerspectiveProjection( float fovy, float aspect, float near, float far )
	{
		ZoneScoped;
		assert( glm::abs( aspect - std::numeric_limits< float >::epsilon() ) > 0 );
		const float tan_half_fovy { std::tan( fovy / 2.0f ) };
		projection_matrix = glm::mat4 { 0.0f };
		projection_matrix[ 0 ][ 0 ] = 1.0f / ( aspect * tan_half_fovy );
		projection_matrix[ 1 ][ 1 ] = 1.0f / ( tan_half_fovy );
		projection_matrix[ 2 ][ 2 ] = far / ( far - near );
		projection_matrix[ 2 ][ 3 ] = 1.0f;
		projection_matrix[ 3 ][ 2 ] = -( far * near ) / ( far - near );

		base_frustum = createFrustum( *this, aspect, fovy, near, far );
	}

	void Camera::setViewDirection( glm::vec3 position, glm::vec3 direction, glm::vec3 up )
	{
		ZoneScoped;
		const glm::vec3 w_direction { glm::normalize( direction ) };
		const glm::vec3 u_right { glm::normalize( glm::cross( w_direction, up ) ) };
		const glm::vec3 v_up { glm::cross( w_direction, u_right ) };

		/*
		 * view_matrix
		 * u_r = view_right
		 * v_u = view_up
		 * w_d = view_direction
		 *
		 * | u_r.x  u_r.y  u_r.z  0 |
		 * | v_u.x  v_u.y  v_u.z  0 |
		 * | w_d.x  w_d.y  w_d.z  0 |
		 * | 0      0      0      1 |
		 */

		view_matrix = glm::mat4 { 1.0f };
		view_matrix[ 0 ][ 0 ] = u_right.x;
		view_matrix[ 1 ][ 0 ] = u_right.y;
		view_matrix[ 2 ][ 0 ] = u_right.z;

		view_matrix[ 0 ][ 1 ] = v_up.x;
		view_matrix[ 1 ][ 1 ] = v_up.y;
		view_matrix[ 2 ][ 1 ] = v_up.z;

		view_matrix[ 0 ][ 2 ] = w_direction.x;
		view_matrix[ 1 ][ 2 ] = w_direction.y;
		view_matrix[ 2 ][ 2 ] = w_direction.z;

		view_matrix[ 3 ][ 0 ] = -glm::dot( u_right, position );
		view_matrix[ 3 ][ 1 ] = -glm::dot( v_up, position );
		view_matrix[ 3 ][ 2 ] = -glm::dot( w_direction, position );

		frustum = base_frustum * view_matrix;
	}

	void Camera::setViewTarget( glm::vec3 position, glm::vec3 target, glm::vec3 up )
	{
		setViewDirection( position, glm::normalize( target - position ), up );
	}

	void Camera::setViewYXZ( glm::vec3 position, glm::vec3 rotation )
	{
		ZoneScoped;
		const float c3 { glm::cos( rotation.z ) };
		const float s3 { glm::sin( rotation.z ) };
		const float c2 { glm::cos( rotation.x ) };
		const float s2 { glm::sin( rotation.x ) };
		const float c1 { glm::cos( rotation.y ) };
		const float s1 { glm::sin( rotation.y ) };
		const glm::vec3 u { ( c1 * c3 + s1 * s2 * s3 ), ( c2 * s3 ), ( c1 * s2 * s3 - c3 * s1 ) };
		const glm::vec3 v { ( c3 * s1 * s2 - c1 * s3 ), ( c2 * c3 ), ( c1 * c3 * s2 + s1 * s3 ) };
		const glm::vec3 w { ( c2 * s1 ), ( -s2 ), ( c1 * c2 ) };

		view_matrix = glm::mat4 { 1.0f };
		view_matrix[ 0 ][ 0 ] = u.x;
		view_matrix[ 1 ][ 0 ] = u.y;
		view_matrix[ 2 ][ 0 ] = u.z;

		view_matrix[ 0 ][ 1 ] = v.x;
		view_matrix[ 1 ][ 1 ] = v.y;
		view_matrix[ 2 ][ 1 ] = v.z;

		view_matrix[ 0 ][ 2 ] = w.x;
		view_matrix[ 1 ][ 2 ] = w.y;
		view_matrix[ 2 ][ 2 ] = w.z;

		view_matrix[ 3 ][ 0 ] = -glm::dot( u, position );
		view_matrix[ 3 ][ 1 ] = -glm::dot( v, position );
		view_matrix[ 3 ][ 2 ] = -glm::dot( w, position );

		frustum = base_frustum * view_matrix;
	}

	Frustum
		createFrustum( const Camera& camera, const float aspect, const float fov_y, const float near, const float far )
	{
		Plane near_plane { camera.getForward(), near };
		Plane far_plane { camera.getBackward(), far };

		const float half_width { near * glm::tan( fov_y / 2.0f ) }; // halfHSide
		const float half_height { half_width / aspect }; //halfVSide

		constexpr glm::vec3 ZERO { 0.0f, 0.0f, 0.0f };

		const auto far_forward { camera.getForward() * far };

		//top_dir is the direction pointing at the highest point on the far plane
		const auto far_up { camera.getUp() * half_height };
		const glm::vec3 top_dir { glm::normalize( far_up + far_forward ) };

		Plane top_plane { glm::cross( top_dir, camera.getUp() ), 0.0f };
		Plane bottom_plane { glm::cross( top_dir, camera.getDown() ), 0.0f };

		const glm::vec3 right_dir { glm::normalize( camera.getRight() * half_width + far_forward ) };
		Plane right_plane { glm::cross( right_dir, camera.getRight() ), 0.0f };
		Plane left_plane { glm::cross( right_dir, camera.getLeft() ), 0.0f };

		return { near_plane, far_plane, top_plane, bottom_plane, right_plane, left_plane };
	}

} // namespace fgl::engine