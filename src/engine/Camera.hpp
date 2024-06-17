//
// Created by kj16609 on 11/28/23.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#pragma GCC diagnostic pop

#include "constants.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/TransformComponent.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	class Camera;

	Frustum< CoordinateSpace::Model >
		createFrustum( const float aspect, const float fovy, const float near, const float far );

	class Camera
	{
#ifdef EXPOSE_CAMERA_INTERNAL

	  public:

#endif

		Matrix< MatrixType::CameraToScreen > projection_matrix { 1.0f };

		Matrix< MatrixType::WorldToCamera > view_matrix { 1.0f };
		glm::mat4 inverse_view_matrix { 1.0f };

		//! Frustum of the camera in model space relative to the camera
		//! @note Must be transformed by the inverse view matrix to get the frustum in world space
		Frustum< CoordinateSpace::Model > base_frustum {};
		Frustum< CoordinateSpace::World > frustum {};
		WorldCoordinate last_frustum_pos { constants::WORLD_CENTER };

		Rotation current_rotation {};

		Matrix< MatrixType::ModelToWorld > frustumTranslationMatrix() const;

		void updateFrustum();

	  public:

		inline Rotation getRotation() const { return current_rotation; }

		inline static TransformComponent frustum_alt_transform { WorldCoordinate( constants::WORLD_CENTER ),
			                                                     glm::vec3( 1.0f ),
			                                                     Rotation() };

		inline static bool update_frustums { true };
		inline static bool update_using_alt { false };

		Camera()
		{
			this->setPerspectiveProjection( 90.0f, 16.0f / 9.0f, constants::NEAR_PLANE, constants::FAR_PLANE );
			this->setView( WorldCoordinate( constants::CENTER ), Rotation( 0.0f, 0.0f, 0.0f ) );
		}

		WorldCoordinate getFrustumPosition() const;

		inline const Frustum< CoordinateSpace::Model >& getBaseFrustum() const { return base_frustum; }

		//! Returns the frustum of the camera in world space
		inline const Frustum< CoordinateSpace::World >& getFrustumBounds() const { return frustum; }

		inline const Matrix< MatrixType::CameraToScreen >& getProjectionMatrix() const { return projection_matrix; }

		inline const Matrix< MatrixType::WorldToCamera >& getViewMatrix() const { return view_matrix; }

		inline Matrix< MatrixType::WorldToScreen > getProjectionViewMatrix() const
		{
			assert( projection_matrix != constants::MAT4_IDENTITY );
			return projection_matrix * view_matrix;
		}

		inline glm::mat4 getInverseViewMatrix() const { return glm::inverse( view_matrix ); }

		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		inline Coordinate< CoordinateSpace::World > getPosition() const
		{
			//Should maybe store the inverse view matrix
			return WorldCoordinate( inverse_view_matrix[ 3 ] );
		}

		inline Vector getUp() const { return -getDown(); }

		inline Vector getRight() const { return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 0 ] ) ) ); }

		inline Vector getForward() const { return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 2 ] ) ) ); }

		inline Vector getLeft() const { return -getRight(); }

		inline Vector getBackward() const { return -getForward(); }

		inline Vector getDown() const { return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 1 ] ) ) ); }

		enum ViewMode
		{
			Euler,
			TaitBryan
		};

		void setView( WorldCoordinate pos, const Rotation rotation, const ViewMode mode = TaitBryan );
	};

} // namespace fgl::engine
