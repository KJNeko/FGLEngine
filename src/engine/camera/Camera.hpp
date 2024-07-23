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

#include "CameraRenderer.hpp"
#include "engine/constants.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/TransformComponent.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/rendering/SwapChain.hpp"

namespace fgl::engine
{
	class CameraSwapchain;
	class Camera;

	Frustum< CoordinateSpace::Model > createFrustum( float aspect, float fovy, float near, float far );

	using CameraIDX = std::uint8_t;

	class Camera
	{
		inline static CameraIDX camera_counter { 0 };
		CameraIDX camera_idx { camera_counter++ };

		Matrix< MatrixType::CameraToScreen > projection_matrix { 1.0f };

		Matrix< MatrixType::WorldToCamera > view_matrix { 1.0f };
		glm::mat4 inverse_view_matrix { 1.0f };

		//! Frustum of the camera in model space relative to the camera
		//! @note Must be transformed by the inverse view matrix to get the frustum in world space
		Frustum< CoordinateSpace::Model > base_frustum {};
		Frustum< CoordinateSpace::World > frustum {};
		WorldCoordinate last_frustum_pos { constants::WORLD_CENTER };

		Rotation current_rotation {};

		vk::Extent2D m_extent;

		inline static std::unique_ptr< CameraRenderer > m_renderer;
		std::shared_ptr< CameraSwapchain > m_swapchain;

		Matrix< MatrixType::ModelToWorld > frustumTranslationMatrix() const;

		void updateFrustum();

	  public:

		static void initCameraRenderer();

		Camera( vk::Extent2D extent );

		void setExtent( vk::Extent2D extent );

		Rotation getRotation() const { return current_rotation; }

		WorldCoordinate getFrustumPosition() const;

		const Frustum< CoordinateSpace::Model >& getBaseFrustum() const { return base_frustum; }

		//! Returns the frustum of the camera in world space
		const Frustum< CoordinateSpace::World >& getFrustumBounds() const { return frustum; }

		const Matrix< MatrixType::CameraToScreen >& getProjectionMatrix() const { return projection_matrix; }

		const Matrix< MatrixType::WorldToCamera >& getViewMatrix() const { return view_matrix; }

		Matrix< MatrixType::WorldToScreen > getProjectionViewMatrix() const;

		glm::mat4 getInverseViewMatrix() const { return glm::inverse( view_matrix ); }

		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		Coordinate< CoordinateSpace::World > getPosition() const;

		Vector getUp() const { return -getDown(); }

		Vector getRight() const { return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 0 ] ) ) ); }

		Vector getForward() const { return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 2 ] ) ) ); }

		Vector getLeft() const { return -getRight(); }

		Vector getBackward() const { return -getForward(); }

		Vector getDown() const { return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 1 ] ) ) ); }

		//! Performs the render pass for this camera
		void pass( FrameInfo& frame_info ) const;

		static vk::raii::RenderPass& getRenderpass();
		CameraSwapchain& getSwapchain() const;
		void setViewport( const vk::raii::CommandBuffer& command_buffer );
		void setScissor( const vk::raii::CommandBuffer& command_buffer );

		void beginRenderpass( const vk::raii::CommandBuffer& command_buffer, const FrameInfo& info );
		void endRenderpass( const vk::raii::CommandBuffer& command_buffer );

		void copyOutput( const vk::raii::CommandBuffer& command_buffer, FrameIndex frame_index, Image& target );

		enum ViewMode
		{
			Euler,
			TaitBryan
		};

		void setView( WorldCoordinate pos, const Rotation& rotation, ViewMode mode = TaitBryan );
	};

} // namespace fgl::engine
