//
// Created by kj16609 on 11/28/23.
//

#include "Camera.hpp"

#include "GameObject.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <cassert>
#include <limits>
#include <utility>

namespace fgl::engine
{

	void Camera::setOrthographicProjection( float left, float right, float top, float bottom, float near, float far )
	{
		ZoneScoped;
		projection_matrix = Matrix< MatrixType::CameraToScreen >( glm::orthoLH( left, right, bottom, top, near, far ) );

		//TODO: Figure out frustum culling for orthographic projection. (If we even wanna use it)
	}

	void Camera::setPerspectiveProjection( float fovy, float aspect, float near, float far )
	{
		ZoneScoped;
		projection_matrix = Matrix< MatrixType::CameraToScreen >( glm::perspectiveLH( fovy, aspect, near, far ) );

		base_frustum = createFrustum( aspect, fovy, near, far );
	}

	void Camera::setViewDirection( glm::vec3 position, const Vector direction, glm::vec3 up )
	{
		ZoneScoped;
		glm::lookAt( position, position + direction, up );
		//frustum = view_matrix * base_frustum;
		return;
	}

	void Camera::setViewTarget( glm::vec3 position, glm::vec3 target, glm::vec3 up )
	{
		setViewDirection( position, Vector( glm::normalize( target - position ) ), up );
	}

	enum RotationOrder
	{
		XZY,
		XYZ,
		YXZ,
		YZX,
		ZYX,
		ZXY,
		END_OF_ENUM,
		DEFAULT = XYZ
	};

	template < int N >
	inline std::tuple< float, float > extract( const Vector rotation, const RotationOrder order )
	{
		switch ( order )
		{
			case XZY:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 2:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 3:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
				}
				break;
			case XYZ:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 2:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 3:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
				}
				break;
			case YXZ:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 2:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 3:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
				}
				break;
			case YZX:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 2:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 3:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
				}
				break;
			case ZYX:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 2:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 3:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
				}
				break;
			case ZXY:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 2:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 3:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
				}
				break;
			case END_OF_ENUM:
				throw std::runtime_error( "Unimplemented rotation order" );
		}
		std::unreachable();
	}

	glm::mat4 taitBryanMatrix( const Vector rotation, const RotationOrder order = DEFAULT )
	{
		glm::mat4 mat { 1.0f };

		const auto [ c1, s1 ] = extract< 1 >( rotation, order );
		const auto [ c2, s2 ] = extract< 2 >( rotation, order );
		const auto [ c3, s3 ] = extract< 3 >( rotation, order );

		switch ( order )
		{
			case RotationOrder::XZY:
				{
					const glm::vec3 row_0 { ( c2 * c3 ), -( s2 ), ( c2 * s3 ) };

					const glm::vec3 row_1 { ( s1 * s3 ) + ( c1 * c3 * s2 ),
						                    ( c1 * c2 ),
						                    ( c1 * s2 * s3 ) - ( c3 * s1 ) };

					const glm::vec3 row_2 { ( c3 * s1 * s2 ) - ( c1 * s3 ),
						                    ( c2 * s1 ),
						                    ( c1 * c3 ) + ( s1 * s2 * s3 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::XYZ:
				{
					const glm::vec3 row_0 { ( c2 * c3 ), -( c2 * s3 ), s2 };
					const glm::vec3 row_1 { ( c1 * s3 ) + ( c3 * s1 * s2 ),
						                    ( c1 * c3 ) - ( s1 * s2 * s3 ),
						                    -( c2 * s1 ) };
					const glm::vec3 row_2 { ( s1 * s3 ) - ( c1 * c3 * s2 ),
						                    ( c3 * s1 ) + ( c1 * s2 * s3 ),
						                    ( c1 * c2 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::YXZ:
				{
					const glm::vec3 row_0 { ( c1 * c3 ) + ( s1 * s2 * s3 ), ( c3 * s1 * s2 ) - ( c1 * s3 ), c2 * s1 };
					const glm::vec3 row_1 { c2 * s3, c2 * c3, -s2 };
					const glm::vec3 row_2 { ( c1 * s2 * s3 ) - ( c3 * s1 ), ( c1 * c3 * s2 ) + ( s1 * s3 ), c1 * c2 };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::YZX:
				{
					const glm::vec3 row_0 { ( c1 * c2 ),
						                    ( s1 * s3 ) - ( c1 * c3 * s2 ),
						                    ( c3 * s1 ) + ( c1 * s2 * s3 ) };
					const glm::vec3 row_1 { s2, c2 * c3, -( c2 * s3 ) };
					const glm::vec3 row_2 { -( c2 * s1 ),
						                    ( c1 * s3 ) + ( c3 * s1 * s2 ),
						                    ( c1 * c3 ) - ( s1 * s2 * s3 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::ZYX: // Roll, Pitch, Yaw
				{
					const glm::vec3 row_0 { ( c1 * c2 ),
						                    ( c1 * s2 * s3 ) - ( c3 * s1 ),
						                    ( s1 * s3 ) + ( c1 * c3 * s2 ) };
					const glm::vec3 row_1 { ( c2 * s1 ),
						                    ( c1 * c3 ) + ( s1 * s2 * s3 ),
						                    ( c3 * s1 * s2 ) - ( c1 * s3 ) };
					const glm::vec3 row_2 { -s2, c2 * s3, c2 * c3 };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::ZXY: // Roll, Yaw, Pitch
				{
					const glm::vec3 row_0 { ( c1 * c3 ) - ( s1 * s2 * s3 ),
						                    -( c2 * s1 ),
						                    ( c1 * s3 ) + ( c3 * s1 * s2 ) };

					const glm::vec3 row_1 { ( c3 * s1 ) + ( c1 * s2 * s3 ),
						                    ( c1 * c2 ),
						                    ( s1 * s3 ) - ( c1 * c3 * s2 ) };

					const glm::vec3 row_2 { -( c2 * s3 ), ( s2 ), ( c2 * c3 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			default:
				throw std::runtime_error( "Unimplemented rotation order" );
		}
	}

	void Camera::setViewYXZ( glm::vec3 position, const Vector rotation, const ViewMode mode )
	{
		ZoneScoped;

		//Flip Z due to the fact we use Z+ outside of this function. It must be Z- inside
		position.z = -position.z;

		switch ( mode )
		{
			case ViewMode::TaitBryan:
				{
					static auto current_rotation_order { RotationOrder::DEFAULT };

					const glm::mat4 rotation_matrix { taitBryanMatrix( rotation, current_rotation_order ) };

					const glm::vec3 forward { rotation_matrix * glm::vec4( constants::WORLD_FORWARD, 0.0f ) };

					const glm::vec3 camera_up { rotation_matrix * glm::vec4( constants::WORLD_UP, 0.0f ) };

					view_matrix =
						Matrix< MatrixType::WorldToCamera >( glm::lookAtLH( position, position + forward, camera_up ) );

					break;
				}
			case ViewMode::Euler:
				{
					//TODO: Implement
					//view_matrix = glm::lookAtLH(position, position + );
				}
			default:
				throw std::runtime_error( "Unimplemented view mode" );
		}

		TransformComponent transform { WorldCoordinate( position ), glm::vec3( 1.0f, 1.0f, 1.0f ), rotation };

		if ( update_frustums )
		{
			if ( update_using_alt ) [[unlikely]]
			{
				frustum = frustum_alt_transform.mat() * base_frustum;
				return;
			}
			else
			{
				frustum = transform.mat() * base_frustum;
				return;
			}
		}
	}

	Frustum< CoordinateSpace::Model >
		createFrustum( const float aspect, const float fov_y, const float near, const float far )
	{
		const Plane< CoordinateSpace::Model > near_plane { constants::WORLD_FORWARD, near };
		const Plane< CoordinateSpace::Model > far_plane { constants::WORLD_BACKWARD, -far };

		const float half_height { far * glm::tan( fov_y / 2.0f ) };
		const float half_width { half_height * aspect };

		const ModelCoordinate far_forward { constants::WORLD_FORWARD * far };
		const ModelCoordinate right_half { constants::WORLD_RIGHT * half_width };

		const Vector right_forward { far_forward + right_half };
		const Vector left_forward { far_forward - right_half };

		const Plane< CoordinateSpace::Model > right_plane { glm::cross( right_forward, constants::WORLD_DOWN ), 0.0f };
		const Plane< CoordinateSpace::Model > left_plane { glm::cross( left_forward, constants::WORLD_UP ), 0.0f };

		const ModelCoordinate top_half { constants::WORLD_UP * half_height };

		const Vector top_forward { far_forward + top_half };
		const Vector bottom_forward { far_forward - top_half };

		const Plane< CoordinateSpace::Model > top_plane { glm::cross( top_forward, constants::WORLD_RIGHT ), 0.0f };

		const Plane< CoordinateSpace::Model > bottom_plane { glm::cross( bottom_forward, constants::WORLD_LEFT ),
			                                                 0.0f };

		std::cout << bottom_plane.direction() << std::endl;

		return { near_plane, far_plane, top_plane, bottom_plane, right_plane, left_plane };
	}

	const Matrix< MatrixType::ModelToWorld > Camera::frustumTranslationMatrix() const
	{
		glm::mat4 translation { 1.0f };

		translation = glm::translate( glm::mat4( 1.0f ), getPosition() );

		return Matrix< MatrixType::ModelToWorld >( translation );
	}

	WorldCoordinate Camera::getFrustumPosition() const
	{
		if ( update_using_alt ) [[unlikely]]
			return frustum_alt_transform.translation;
		else
			return getPosition();
	}

} // namespace fgl::engine