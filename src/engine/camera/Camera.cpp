//
// Created by kj16609 on 11/28/23.
//

#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <tracy/Tracy.hpp>

#include "CameraSwapchain.hpp"

namespace fgl::engine
{

	void Camera::setOrthographicProjection( float left, float right, float top, float bottom, float near, float far )
	{
		projection_matrix =
			Matrix< MatrixType::CameraToScreen >( glm::orthoLH_ZO( left, right, bottom, top, near, far ) );

		//TODO: Figure out frustum culling for orthographic projection. (If we even wanna use it)
	}

	FGL_FLATTEN_HOT void Camera::setPerspectiveProjection( float fovy, float aspect, float near, float far )
	{
		projection_matrix = Matrix< MatrixType::CameraToScreen >( glm::perspectiveLH_ZO( fovy, aspect, near, far ) );

		base_frustum = createFrustum( aspect, fovy, near, far );
	}

	FGL_FLATTEN_HOT void Camera::setView( WorldCoordinate pos, const Rotation rotation, const ViewMode mode )
	{
		switch ( mode )
		{
			case ViewMode::TaitBryan:
				{
					const RotationMatrix rotation_matrix { rotation.mat() };

					const glm::vec3 forward { rotation_matrix * glm::vec4( constants::WORLD_FORWARD, 0.0f ) };

					const glm::vec3 camera_up { rotation_matrix * glm::vec4( constants::WORLD_UP, 0.0f ) };

					const WorldCoordinate center_pos { pos + forward };

					view_matrix =
						Matrix< MatrixType::WorldToCamera >( glm::lookAtLH( pos.vec(), center_pos.vec(), -camera_up ) );

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
		last_frustum_pos = getPosition();

		const Matrix< MatrixType::ModelToWorld > translation_matrix { frustumTranslationMatrix() };

		frustum = translation_matrix * base_frustum;
	}

	Camera::Camera( const vk::Extent2D extent ) :
	  m_extent( extent ),
	  m_swapchain( std::make_shared< CameraSwapchain >( m_extent ) )
	{
		this->setPerspectiveProjection( 90.0f, 16.0f / 9.0f, constants::NEAR_PLANE, constants::FAR_PLANE );
		this->setView( WorldCoordinate( constants::CENTER ), Rotation( 0.0f, 0.0f, 0.0f ) );
	}

	void Camera::setExtent( const vk::Extent2D extent )
	{
		m_extent = extent;
		m_swapchain = std::make_shared< CameraSwapchain >( m_extent );
	}

	Frustum< CoordinateSpace::Model >
		createFrustum( const float aspect, const float fov_y, const float near, const float far )
	{
		const Plane< CoordinateSpace::Model > near_plane { ModelCoordinate( constants::WORLD_FORWARD * near ),
			                                               NormalVector::bypass( constants::WORLD_FORWARD ) };
		const Plane< CoordinateSpace::Model > far_plane { ModelCoordinate( constants::WORLD_FORWARD * far ),
			                                              NormalVector::bypass( constants::WORLD_BACKWARD ) };

		const float half_height { far * glm::tan( fov_y / 2.0f ) };
		const float half_width { half_height * aspect };

		const ModelCoordinate far_forward { constants::WORLD_FORWARD * far };
		const ModelCoordinate right_half { constants::WORLD_RIGHT * half_width };

		const Vector right_forward { ( far_forward + right_half ).vec() };
		const Vector left_forward { ( far_forward - right_half ).vec() };

		const Plane< CoordinateSpace::Model > right_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( right_forward.vec(), constants::WORLD_DOWN ) )
		};
		const Plane< CoordinateSpace::Model > left_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( left_forward.vec(), constants::WORLD_UP ) )
		};

		const ModelCoordinate top_half { constants::WORLD_UP * half_height };

		const Vector top_forward { ( far_forward + top_half ).vec() };
		const Vector bottom_forward { ( far_forward - top_half ).vec() };

		const Plane< CoordinateSpace::Model > top_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( top_forward.vec(), constants::WORLD_RIGHT ) )
		};

		const Plane< CoordinateSpace::Model > bottom_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( bottom_forward.vec(), constants::WORLD_LEFT ) )
		};

		return { near_plane,
			     far_plane,
			     top_plane,
			     bottom_plane,
			     right_plane,
			     left_plane,
			     Coordinate< CoordinateSpace::Model >( constants::WORLD_CENTER ) };
	}

	Matrix< MatrixType::ModelToWorld > Camera::frustumTranslationMatrix() const
	{
		TransformComponent comp {};
		comp.translation = getPosition();
		comp.rotation = current_rotation;

		return comp.mat();
	}

	WorldCoordinate Camera::getFrustumPosition() const
	{
		return last_frustum_pos;
	}

} // namespace fgl::engine
