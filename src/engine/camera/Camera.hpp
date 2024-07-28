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
#include "engine/buffers/HostSingleT.hpp"
#include "engine/buffers/UniqueFrameSuballocation.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Rotation.hpp"
#include "engine/rendering/types.hpp"

namespace vk::raii
{
	class CommandBuffer;
	class RenderPass;
} // namespace vk::raii

namespace fgl::engine
{
	class Image;
	struct FrameInfo;
	class CameraRenderer;

	struct CameraInfo;
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

		PerFrameSuballocation< HostSingleT< CameraInfo > > m_camera_frame_info;

		// Camera info is expected at binding 0
		std::vector< descriptors::DescriptorSet > m_camera_info_descriptors {};

		inline static std::unique_ptr< CameraRenderer > m_renderer;
		std::shared_ptr< CameraSwapchain > m_swapchain;

		Matrix< MatrixType::ModelToWorld > frustumTranslationMatrix() const;

		void updateFrustum();

		Camera( vk::Extent2D extent, memory::Buffer& data_buffer );

		friend class CameraManager;

	  public:

		FGL_DELETE_ALL_Ro5( Camera );

		~Camera();

		CameraIDX getIDX() { return camera_idx; }

		static void initCameraRenderer();

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

		enum ViewMode
		{
			Euler,
			TaitBryan
		};

		void setView( WorldCoordinate pos, const Rotation& rotation, ViewMode mode = TaitBryan );
		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		Coordinate< CoordinateSpace::World > getPosition() const;

		FGL_FORCE_INLINE Vector getUp() const { return -getDown(); }

		FGL_FORCE_INLINE Vector getRight() const
		{
			return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 0 ] ) ) );
		}

		FGL_FORCE_INLINE Vector getForward() const
		{
			return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 2 ] ) ) );
		}

		FGL_FORCE_INLINE Vector getLeft() const { return -getRight(); }

		FGL_FORCE_INLINE Vector getBackward() const { return -getForward(); }

		FGL_FORCE_INLINE Vector getDown() const
		{
			return Vector( glm::normalize( glm::vec3( inverse_view_matrix[ 1 ] ) ) );
		}

		//! Updates the required info for rendering
		void updateInfo( FrameIndex frame_index );
		descriptors::DescriptorSet& getDescriptor( FrameIndex index );

		//! Performs the render pass for this camera
		void pass( FrameInfo& frame_info );

		static vk::raii::RenderPass& getRenderpass();
		CameraSwapchain& getSwapchain() const;
		void setViewport( const vk::raii::CommandBuffer& command_buffer );
		void setScissor( const vk::raii::CommandBuffer& command_buffer );

		void beginRenderpass( const vk::raii::CommandBuffer& command_buffer, const FrameInfo& info );
		void endRenderpass( const vk::raii::CommandBuffer& command_buffer );

		void copyOutput( const vk::raii::CommandBuffer& command_buffer, FrameIndex frame_index, Image& target );
	};

} // namespace fgl::engine
