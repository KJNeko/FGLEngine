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

#include "engine/memory/buffers/HostSingleT.hpp"
#include "engine/memory/buffers/UniqueFrameSuballocation.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Rotation.hpp"
#include "engine/primitives/TransformComponent.hpp"
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

	FrustumBase createFrustum( float aspect, float fovy, float near, float far );

	using CameraIDX = std::uint8_t;

	class Camera
	{
		inline static CameraIDX camera_counter { 0 };

		// Const is acceptable, Since this value should never change. EVER
		const CameraIDX camera_idx { camera_counter++ };

		Matrix< MatrixType::CameraToScreen > projection_matrix { 1.0f };

		Matrix< MatrixType::WorldToCamera > view_matrix { 1.0f };
		glm::mat4 inverse_view_matrix { 1.0f };

		//! Frustum of the camera in model space relative to the camera
		//! @note Must be transformed by the inverse view matrix to get the frustum in world space
		FrustumBase base_frustum {};
		Frustum frustum {};
		WorldCoordinate last_frustum_pos { constants::WORLD_CENTER };

		WorldTransform m_transform;

		vk::Extent2D m_target_extent;
		float m_fov_y { glm::radians( 90.0f ) };

		PerFrameSuballocation< HostSingleT< CameraInfo > > m_camera_frame_info;

		// Camera info is expected at binding 0
		std::vector< descriptors::DescriptorSet > m_camera_info_descriptors {};

		// TODO: Remove this old swapchain and instead do a proper deffered cleanup of it.
		std::shared_ptr< CameraSwapchain > m_old_swapchain { nullptr };
		std::shared_ptr< CameraSwapchain > m_swapchain;

		std::string name;

		Matrix< MatrixType::ModelToWorld > frustumTranslationMatrix() const;

		void updateFrustum();

#ifdef EXPOSE_CAMERA_TESTS
		//Constructor for tests
		Camera( vk::Extent2D test_extent ) : m_target_extent( test_extent ) {}
#endif

		Camera( vk::Extent2D extent, memory::Buffer& data_buffer );

		friend class CameraManager;

	  public:

		float& x { m_transform.translation.x };
		float& y { m_transform.translation.y };
		float& z { m_transform.translation.z };

		FGL_DELETE_ALL_Ro5( Camera );

		~Camera();

		CameraIDX getIDX() const;

		const std::string& getName() const;

		static void initCameraRenderer();

		void setExtent( vk::Extent2D extent );

		const Rotation& getRotation() const { return m_transform.rotation; }

		Rotation& getRotation() { return m_transform.rotation; }

		const WorldTransform& getTransform() const { return m_transform; }

		WorldTransform& getTransform() { return m_transform; }

		WorldCoordinate getFrustumPosition() const;

		const FrustumBase& getBaseFrustum() const { return base_frustum; }

		//! Returns the frustum of the camera in world space
		const Frustum& getFrustumBounds() const { return frustum; }

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

		void setFOV( const float fov_y );

		//! Performs the render pass for this camera
		void pass( FrameInfo& frame_info );

		static vk::raii::RenderPass& getRenderpass();
		CameraSwapchain& getSwapchain() const;
		void setViewport( const vk::raii::CommandBuffer& command_buffer );
		void setScissor( const vk::raii::CommandBuffer& command_buffer );

		void remakeSwapchain( vk::Extent2D extent );

		void setName( std::string_view str );

		float aspectRatio() const;

		void copyOutput( const vk::raii::CommandBuffer& command_buffer, FrameIndex frame_index, Image& target );
		void updateMatrix();

#ifdef EXPOSE_CAMERA_TESTS

		Camera CREATE_TESTING_CAMERA() { return { { 1920, 1080 } }; }

#endif
	};

} // namespace fgl::engine
