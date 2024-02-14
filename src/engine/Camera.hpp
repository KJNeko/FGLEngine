//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <glm/glm.hpp>

#include "constants.hpp"
#include "engine/primitives/Coordinate.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Matrix.hpp"

namespace fgl::engine
{
	class Camera;

	Frustum< CoordinateSpace::Model >
		createFrustum( const Camera& camera, const float aspect, const float fovy, const float near, const float far );

	class Camera
	{
		Matrix< MatrixType::CameraToScreen > projection_matrix { 1.0f };

		Matrix< MatrixType::WorldToCamera > view_matrix { 1.0f };

		//! Frustum of the camera in model space relative to the camera
		//! @note Must be transformed by the inverse view matrix to get the frustum in world space
		Frustum< CoordinateSpace::Model > base_frustum {};
		Frustum< CoordinateSpace::World > frustum {};

		friend Frustum< CoordinateSpace::Model > createFrustum(
			const Camera& camera, const float aspect, const float fovy, const float near, const float far );

		const Matrix< MatrixType::ModelToWorld > frustumTranslationMatrix() const;

	  public:

		const Frustum< CoordinateSpace::Model >& getBaseFrustum() const { return base_frustum; }

		//! Returns the frustum of the camera in world space
		const Frustum< CoordinateSpace::World >& getFrustumBounds() const { return frustum; }

		const Matrix< MatrixType::CameraToScreen >& getProjectionMatrix() const { return projection_matrix; }

		const Matrix< MatrixType::WorldToCamera > getViewMatrix() const { return view_matrix; }

		const Matrix< MatrixType::WorldToScreen > getProjectionViewMatrix() const
		{
			return projection_matrix * view_matrix;
		}

		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		const Coordinate< CoordinateSpace::World > getPosition() const { return WorldCoordinate( -view_matrix[ 3 ] ); }

		const Vector getUp() const
		{
			return Vector(
				glm::normalize( glm::vec3( view_matrix[ 0 ][ 1 ], view_matrix[ 1 ][ 1 ], view_matrix[ 2 ][ 1 ] ) ) );
		}

		const Vector getRight() const
		{
			return Vector(
				glm::normalize( glm::vec3( view_matrix[ 0 ][ 0 ], view_matrix[ 1 ][ 0 ], view_matrix[ 2 ][ 0 ] ) ) );
		}

		const Vector getForward() const
		{
			return Vector(
				glm::normalize( glm::vec3( view_matrix[ 0 ][ 2 ], view_matrix[ 1 ][ 2 ], view_matrix[ 2 ][ 2 ] ) ) );
		}

		const Vector getLeft() const { return -getRight(); }

		const Vector getBackward() const { return -getForward(); }

		const Vector getDown() const { return -getUp(); }

		void setViewDirection( glm::vec3 pos, const Vector direction, glm::vec3 up = constants::WORLD_UP );
		void setViewTarget( glm::vec3 pos, glm::vec3 target, glm::vec3 up = constants::WORLD_UP );

		enum ViewMode
		{
			Euler,
			TaitBryan
		};

		void setViewYXZ( glm::vec3 pos, const Vector rotation, const ViewMode mode = TaitBryan );
	};

} // namespace fgl::engine
