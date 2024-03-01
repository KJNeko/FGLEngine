//
// Created by kj16609 on 11/28/23.
//

#include "Camera.hpp"

#include "GameObject.hpp"
#include "engine/math/taitBryanMatrix.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <tracy/Tracy.hpp>

namespace fgl::engine
{

	void Camera::setOrthographicProjection( float left, float right, float top, float bottom, float near, float far )
	{
		projection_matrix =
			Matrix< MatrixType::CameraToScreen >( glm::orthoLH_ZO( left, right, bottom, top, near, far ) );

		//TODO: Figure out frustum culling for orthographic projection. (If we even wanna use it)
	}

	void Camera::setPerspectiveProjection( float fovy, float aspect, float near, float far )
	{
		projection_matrix = Matrix< MatrixType::CameraToScreen >( glm::perspectiveLH_ZO( fovy, aspect, near, far ) );

		base_frustum = createFrustum( aspect, fovy, near, far );
	}

	void Camera::setViewDirection( glm::vec3 position, const Vector direction, glm::vec3 up )
	{
		glm::lookAt( position, position + direction.vec(), up );
		//frustum = view_matrix * base_frustum;
		return;
	}

	void Camera::setViewTarget( glm::vec3 position, glm::vec3 target, glm::vec3 up )
	{
		setViewDirection( position, Vector( glm::normalize( target - position ) ), up );
	}

	void Camera::setView( WorldCoordinate pos, const Rotation rotation, const ViewMode mode )
	{
		//Flip Z due to the fact we use Z+ outside of this function. It must be Z- inside
		//position.z = -position.z;
		// Maybe unneeded?

		switch ( mode )
		{
			case ViewMode::TaitBryan:
				{
					const auto rotation_matrix { rotation.mat() };

					const glm::vec3 forward { rotation_matrix * glm::vec4( constants::WORLD_FORWARD, 0.0f ) };

					const glm::vec3 camera_up { rotation_matrix * glm::vec4( constants::WORLD_UP, 0.0f ) };

					view_matrix = Matrix<
						MatrixType::WorldToCamera >( glm::lookAtLH( pos.vec(), ( pos + forward ).vec(), -camera_up ) );
					inverse_view_matrix = glm::inverse( view_matrix );

					break;
				}
			case ViewMode::Euler:
				[[fallthrough]];
				{
					//TODO: Implement
					//view_matrix = glm::lookAtLH(position, position + );
				}
			default:
				throw std::runtime_error( "Unimplemented view mode" );
		}

		current_rotation = rotation;

		updateFrustum();
	}

	void Camera::updateFrustum()
	{
		if ( update_frustums ) [[likely]]
		{
			last_frustum_pos = getPosition();

			frustum = frustumTranslationMatrix() * base_frustum;
			return;
		}
		else
			return;
	}

	Frustum< CoordinateSpace::Model >
		createFrustum( const float aspect, const float fov_y, const float near, const float far )
	{
		const Plane< CoordinateSpace::Model > near_plane { constants::WORLD_FORWARD * near, constants::WORLD_FORWARD };
		const Plane< CoordinateSpace::Model > far_plane { constants::WORLD_FORWARD * far, constants::WORLD_BACKWARD };

		const float half_height { far * glm::tan( fov_y / 2.0f ) };
		const float half_width { half_height * aspect };

		const ModelCoordinate far_forward { constants::WORLD_FORWARD * far };
		const ModelCoordinate right_half { constants::WORLD_RIGHT * half_width };

		const Vector right_forward { far_forward + right_half };
		const Vector left_forward { far_forward - right_half };

		const Plane< CoordinateSpace::Model > right_plane { constants::WORLD_CENTER,
			                                                glm::cross( right_forward.vec(), constants::WORLD_DOWN ) };
		const Plane< CoordinateSpace::Model > left_plane { constants::WORLD_CENTER,
			                                               glm::cross( left_forward.vec(), constants::WORLD_UP ) };

		const ModelCoordinate top_half { constants::WORLD_UP * half_height };

		const Vector top_forward { far_forward + top_half };
		const Vector bottom_forward { far_forward - top_half };

		const Plane< CoordinateSpace::Model > top_plane { constants::WORLD_CENTER,
			                                              glm::cross( top_forward.vec(), constants::WORLD_RIGHT ) };

		const Plane< CoordinateSpace::Model > bottom_plane {
			constants::WORLD_CENTER, glm::cross( bottom_forward.vec(), constants::WORLD_LEFT )
		};

		return { near_plane,
			     far_plane,
			     top_plane,
			     bottom_plane,
			     right_plane,
			     left_plane,
			     Coordinate< CoordinateSpace::Model >( constants::WORLD_CENTER ) };
	}

	const Matrix< MatrixType::ModelToWorld > Camera::frustumTranslationMatrix() const
	{
		if ( update_using_alt )
			return frustum_alt_transform.mat();
		else [[likely]]
		{
			TransformComponent comp;
			comp.translation = getPosition();
			comp.rotation = current_rotation;

			return comp.mat();
		}
	}

	WorldCoordinate Camera::getFrustumPosition() const
	{
		if ( update_using_alt ) [[unlikely]]
			return frustum_alt_transform.translation;
		else if ( update_frustums ) [[likely]]
			return getPosition();
		else
			return last_frustum_pos;
	}

} // namespace fgl::engine