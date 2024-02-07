//
// Created by kj16609 on 11/28/23.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "engine/coordinates/WorldCoordinate.hpp"

namespace fgl::engine
{
	constexpr static auto WORLD_UP { glm::vec3 { 0.0f, 1.0f, 0.0f } };

	class Camera;

	Frustum
		createFrustum( const Camera& camera, const float aspect, const float fovy, const float near, const float far );

	class Camera
	{
		glm::mat4 projection_matrix { 1.0f };

		glm::mat4 view_matrix { 1.0f };

		//! Frustum of the camera in model space relative to the camera
		//! @note Must be transformed by the inverse view matrix to get the frustum in world space
		Frustum base_frustum {};
		Frustum frustum {};

		friend Frustum createFrustum(
			const Camera& camera, const float aspect, const float fovy, const float near, const float far );

	  public:

		//! Returns the frustum of the camera in world space
		const Frustum& getFrustumBounds() const { return frustum; }

		const glm::mat4& getProjectionMatrix() const { return projection_matrix; }

		const glm::mat4& getViewMatrix() const { return view_matrix; }

		const glm::mat4 getProjectionViewMatrix() const { return projection_matrix * view_matrix; }

		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		const glm::vec3 getPosition() const { return glm::vec3( -view_matrix[ 3 ] ); }

		const glm::vec3 getUp() const
		{
			return glm::normalize( glm::vec3( view_matrix[ 0 ][ 1 ], view_matrix[ 1 ][ 1 ], view_matrix[ 2 ][ 1 ] ) );
		}

		const glm::vec3 getRight() const
		{
			return glm::normalize( glm::vec3( view_matrix[ 0 ][ 0 ], view_matrix[ 1 ][ 0 ], view_matrix[ 2 ][ 0 ] ) );
		}

		const glm::vec3 getForward() const
		{
			return glm::normalize( glm::vec3( view_matrix[ 0 ][ 2 ], view_matrix[ 1 ][ 2 ], view_matrix[ 2 ][ 2 ] ) );
		}

		const glm::vec3 getLeft() const { return -getRight(); }

		const glm::vec3 getBackward() const { return -getForward(); }

		const glm::vec3 getDown() const { return -getUp(); }

		void setViewDirection( glm::vec3 pos, glm::vec3 direction, glm::vec3 up = constants::WORLD_UP );
		void setViewTarget( glm::vec3 pos, glm::vec3 target, glm::vec3 up = constants::WORLD_UP );

		enum ViewMode
		{
			Euler,
			TaitBryan
		};

		void setViewYXZ( glm::vec3 pos, glm::vec3 rotation, const ViewMode mode = TaitBryan );
	};

} // namespace fgl::engine
