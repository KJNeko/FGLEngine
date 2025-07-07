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

#include "CompositeSwapchain.hpp"
#include "GBufferSwapchain.hpp"
#include "debug/Track.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/memory/buffers/HostSingleT.hpp"
#include "engine/memory/buffers/UniqueFrameSuballocation.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Transform.hpp"
#include "engine/rendering/types.hpp"

namespace vk::raii
{
	class CommandBuffer;
	class RenderPass;
} // namespace vk::raii

namespace fgl::engine
{
	namespace descriptors
	{
		class DescriptorSetLayout;
	}
	class Image;
	struct FrameInfo;
	class GBufferRenderer;

	struct CameraInfo;
	class Camera;

	FrustumBase createFrustum( float aspect, float fovy, float near, float far );

	using CameraIDX = std::uint8_t;

	class Camera
	{
		inline static CameraIDX m_camera_counter { 0 };

		debug::Track< "CPU", "Camera" > m_camera {};

		vk::Extent2D m_target_extent;

		std::unique_ptr< CompositeSwapchain > m_composite_swapchain;
		std::unique_ptr< GBufferSwapchain > m_gbuffer_swapchain;

		//TODO: Move to deffered deleter
		std::queue< std::unique_ptr< CompositeSwapchain > > m_old_composite_swapchain {};
		std::queue< std::unique_ptr< GBufferSwapchain > > m_old_gbuffer_swapchain {};

		std::shared_ptr< GBufferRenderer > m_camera_renderer;

		//! True if the camera is active and to be rendered
		bool m_active { true };

		//! If true, The camera's swapchain is to be destroyed in order to preserve memory.
		//! This is here to allow us to set a camera cold when it's not likely to be used soon
		bool m_cold { false };

		// Const is acceptable, Since this value should never change. EVER
		const CameraIDX m_camera_idx { m_camera_counter++ };

		Matrix< MatrixType::CameraToScreen > m_projection_matrix { 1.0f };

		Matrix< MatrixType::WorldToCamera > m_view_matrix { 1.0f };
		glm::mat4 m_inverse_view_matrix { 1.0f };

		//! Frustum of the camera in model space relative to the camera
		//! @note Must be transformed by the inverse view matrix to get the frustum in world space
		FrustumBase m_base_frustum {};
		Frustum m_frustum {};
		WorldCoordinate m_last_frustum_pos { constants::WORLD_CENTER };

		WorldTransform m_transform {};

		float m_fov_y { glm::radians( 90.0f ) };

		PerFrameSuballocation< HostSingleT< CameraInfo > > m_camera_frame_info;

		// Camera info is expected at binding 0
		std::vector< descriptors::DescriptorSetPtr > createCameraDescriptors();
		std::vector< descriptors::DescriptorSetPtr > m_camera_info_descriptors;

		std::string m_name { "Unnamed Camera" };

		[[nodiscard]] Matrix< MatrixType::ModelToWorld > frustumTranslationMatrix() const;

		void updateFrustum();

#ifdef EXPOSE_CAMERA_TESTS
		//Constructor for tests
		Camera( vk::Extent2D test_extent ) : m_target_extent( test_extent ) {}
#endif

		Camera( vk::Extent2D extent, memory::Buffer& buffer, const std::shared_ptr< GBufferRenderer >& renderer );

		friend class CameraManager;

	  public:

		float& x { m_transform.translation.x };
		float& y { m_transform.translation.y };
		float& z { m_transform.translation.z };

		FGL_DELETE_ALL_RO5( Camera );

		~Camera();

		[[nodiscard]] CameraIDX getIDX() const;

		[[nodiscard]] const std::string& getName() const;

		void setExtent( vk::Extent2D extent );

		[[nodiscard]] QuatRotation getRotation() const { return m_transform.rotation.forcedQuat(); }

		[[nodiscard]] const WorldTransform& getTransform() const { return m_transform; }

		WorldTransform& getTransform() { return m_transform; }

		WorldCoordinate getFrustumPosition() const;

		[[nodiscard]] const FrustumBase& getBaseFrustum() const { return m_base_frustum; }

		//! Returns the frustum of the camera in world space
		[[nodiscard]] const Frustum& getFrustumBounds() const { return m_frustum; }

		[[nodiscard]] const Matrix< MatrixType::CameraToScreen >& getProjectionMatrix() const
		{
			return m_projection_matrix;
		}

		[[nodiscard]] const Matrix< MatrixType::WorldToCamera >& getViewMatrix() const { return m_view_matrix; }

		[[nodiscard]] Matrix< MatrixType::WorldToScreen > getProjectionViewMatrix() const;

		[[nodiscard]] glm::mat4 getInverseViewMatrix() const;

		enum ViewMode
		{
			Euler,
			TaitBryan
		};

		void setView( WorldCoordinate pos, const QuatRotation& rotation, ViewMode mode = TaitBryan );
		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		[[nodiscard]] Coordinate< CoordinateSpace::World > getPosition() const;

		FGL_FORCE_INLINE NormalVector getUp() const { return -getDown(); }

		FGL_FORCE_INLINE NormalVector getRight() const
		{
			return NormalVector( glm::vec3( m_inverse_view_matrix[ 0 ] ) );
		}

		FGL_FORCE_INLINE NormalVector getForward() const
		{
			return -NormalVector( glm::vec3( m_inverse_view_matrix[ 2 ] ) );
		}

		FGL_FORCE_INLINE NormalVector getLeft() const { return -getRight(); }

		FGL_FORCE_INLINE NormalVector getBackward() const { return -getForward(); }

		FGL_FORCE_INLINE NormalVector getDown() const
		{
			return NormalVector( glm::vec3( m_inverse_view_matrix[ 1 ] ) );
		}

		//! Updates the required info for rendering
		void updateInfo( FrameIndex frame_index );
		descriptors::DescriptorSet& getDescriptor( FrameIndex index );

		void setFOV( float fov_y );

		//! Performs the render pass for this camera
		void pass( FrameInfo& frame_info );

		[[nodiscard]] GBufferSwapchain& getSwapchain() const;
		[[nodiscard]] CompositeSwapchain& getCompositeSwapchain() const;
		void setViewport( const vk::raii::CommandBuffer& command_buffer );
		void setScissor( const vk::raii::CommandBuffer& command_buffer );

		void remakeSwapchain( vk::Extent2D extent );

		void setName( std::string_view str );

		float aspectRatio() const;

		void copyOutput( const vk::raii::CommandBuffer& command_buffer, FrameIndex frame_index, Image& target );
		void updateMatrix();

		static descriptors::DescriptorSetLayout& getDescriptorLayout();

#ifdef EXPOSE_CAMERA_TESTS

		Camera CREATE_TESTING_CAMERA() { return { { 1920, 1080 } }; }

#endif
	};

} // namespace fgl::engine
