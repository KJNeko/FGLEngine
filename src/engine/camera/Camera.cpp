//
// Created by kj16609 on 11/28/23.
//

#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <tracy/Tracy.hpp>

#include "CameraDescriptor.hpp"
#include "CameraInfo.hpp"
#include "CameraRenderer.hpp"
#include "CameraSwapchain.hpp"

namespace fgl::engine
{

	Matrix< MatrixType::WorldToScreen > Camera::getProjectionViewMatrix() const
	{
		assert( projection_matrix != constants::MAT4_IDENTITY );
		return projection_matrix * view_matrix;
	}

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

	Coordinate< CoordinateSpace::World > Camera::getPosition() const
	{
		//Should maybe store the inverse view matrix
		return WorldCoordinate( inverse_view_matrix[ 3 ] );
	}

	void Camera::updateInfo( const FrameIndex frame_index )
	{
		CameraInfo current_camera_info { .projection = getProjectionMatrix(),
			                             .view = getViewMatrix(),
			                             .inverse_view = getInverseViewMatrix() };

		m_camera_frame_info[ frame_index ] = current_camera_info;
	}

	descriptors::DescriptorSet& Camera::getDescriptor( const FrameIndex index )
	{
		assert( index < m_camera_info_descriptors.size() );
		return m_camera_info_descriptors[ index ];
	}

	void Camera::pass( FrameInfo& frame_info )
	{
		assert( frame_info.camera_data.camera == nullptr );
		frame_info.camera_data.camera = this;
		updateInfo( frame_info.frame_idx );
		m_renderer->pass( frame_info, *m_swapchain );
		frame_info.camera_data.camera = nullptr;
	}

	vk::raii::RenderPass& Camera::getRenderpass()
	{
		return m_renderer->getRenderpass();
	}

	CameraSwapchain& Camera::getSwapchain() const
	{
		return *m_swapchain;
	}

	void Camera::setViewport( const vk::raii::CommandBuffer& command_buffer )
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;

		const auto [ width, height ] = m_extent;
		viewport.width = static_cast< float >( width );
		viewport.height = static_cast< float >( height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const std::vector< vk::Viewport > viewports { viewport };

		command_buffer.setViewport( 0, viewports );
	}

	void Camera::setScissor( const vk::raii::CommandBuffer& command_buffer )
	{
		const vk::Rect2D scissor { { 0, 0 }, m_extent };

		const std::vector< vk::Rect2D > scissors { scissor };

		command_buffer.setScissor( 0, scissors );
	}

	void Camera::beginRenderpass( const vk::raii::CommandBuffer& command_buffer, const FrameInfo& info )
	{
		vk::RenderPassBeginInfo begin_info {};
		begin_info.renderPass = getRenderpass();
		begin_info.framebuffer = this->getSwapchain().getFramebuffer( info.frame_idx );
		begin_info.renderArea = { .offset = { 0, 0 }, .extent = m_extent };

		begin_info.setClearValues( this->getSwapchain().getClearValues() );

		command_buffer.beginRenderPass( begin_info, vk::SubpassContents::eInline );

		setViewport( command_buffer );
		setScissor( command_buffer );
	}

	void Camera::endRenderpass( const vk::raii::CommandBuffer& command_buffer )
	{
		command_buffer.endRenderPass();
	}

	void Camera::
		copyOutput( const vk::raii::CommandBuffer& command_buffer, const FrameIndex frame_index, Image& target )
	{
		assert( m_extent == target.getExtent() );

		Image& source { this->getSwapchain().getOutput( frame_index ) };

		vk::ImageSubresourceRange range {};
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier barrier_to_target {};
		barrier_to_target.oldLayout = vk::ImageLayout::eUndefined;
		barrier_to_target.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier_to_target.image = target.getVkImage();
		barrier_to_target.subresourceRange = range;
		barrier_to_target.srcAccessMask = vk::AccessFlagBits::eMemoryWrite;
		barrier_to_target.dstAccessMask =
			vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eColorAttachmentWrite;
		barrier_to_target.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier_to_target.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{},
			{},
			{},
			{ barrier_to_target } );

		vk::ImageMemoryBarrier barrier_from_source {};
		barrier_from_source.oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier_from_source.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier_from_source.image = source.getVkImage();
		barrier_from_source.subresourceRange = range;
		barrier_from_source.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		barrier_from_source.dstAccessMask = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
		barrier_from_source.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier_from_source.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			{},
			{},
			{ barrier_from_source } );

		vk::ImageCopy region {};
		region.extent = vk::Extent3D( m_extent, 1 );

		region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.srcSubresource.layerCount = 1;
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = 0;

		region.srcOffset = vk::Offset3D( 0, 0, 0 );

		region.dstOffset = region.srcOffset;
		region.dstSubresource = region.srcSubresource;

		command_buffer.copyImage(
			source.getVkImage(),
			vk::ImageLayout::eTransferSrcOptimal,
			target.getVkImage(),
			vk::ImageLayout::eTransferDstOptimal,
			{ region } );

		vk::ImageMemoryBarrier barrier_from_target {};

		barrier_from_target.oldLayout = barrier_to_target.newLayout;
		barrier_from_target.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier_from_target.image = target.getVkImage();
		barrier_from_target.subresourceRange = range;
		barrier_from_target.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier_from_target.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead
		                                  | vk::AccessFlagBits::eColorAttachmentRead;
		barrier_from_target.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier_from_target.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{},
			{},
			{},
			{ barrier_from_target } );

		vk::ImageMemoryBarrier barrier_to_source {};
		barrier_to_source.oldLayout = barrier_from_source.newLayout;
		barrier_to_source.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier_to_source.image = source.getVkImage();
		barrier_to_source.subresourceRange = range;
		barrier_to_source.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier_to_source.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier_to_source.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier_to_source.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{},
			{},
			{},
			{ barrier_to_source } );
	}

	FGL_FLATTEN_HOT void Camera::setView( WorldCoordinate pos, const Rotation& rotation, const ViewMode mode )
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

	void Camera::initCameraRenderer()
	{
		assert( !m_renderer );
		m_renderer = std::make_unique< CameraRenderer >();
	}

	Camera::Camera( const vk::Extent2D extent, memory::Buffer& buffer ) :
	  m_extent( extent ),
	  m_camera_frame_info( buffer, SwapChain::MAX_FRAMES_IN_FLIGHT ),
	  m_swapchain( std::make_shared< CameraSwapchain >( m_renderer->getRenderpass(), m_extent ) )
	{
		this->setPerspectiveProjection( 90.0f, 16.0f / 9.0f, constants::NEAR_PLANE, constants::FAR_PLANE );
		this->setView( WorldCoordinate( constants::CENTER ), Rotation( 0.0f, 0.0f, 0.0f ) );

		for ( std::uint8_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			descriptors::DescriptorSet set { CameraDescriptorSet::createLayout() };
			set.setMaxIDX( 0 );
			set.bindUniformBuffer( 0, m_camera_frame_info[ i ] );
			set.update();

			m_camera_info_descriptors.emplace_back( std::move( set ) );
		}
	}

	void Camera::setExtent( const vk::Extent2D extent )
	{
		m_extent = extent;
		m_swapchain = std::make_shared< CameraSwapchain >( m_renderer->getRenderpass(), m_extent );
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

	Camera::~Camera()
	{}

} // namespace fgl::engine
