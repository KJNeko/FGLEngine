//
// Created by kj16609 on 11/28/23.
//

#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <tracy/Tracy.hpp>

#include "CameraInfo.hpp"
#include "GBufferRenderer.hpp"
#include "GBufferSwapchain.hpp"
#include "engine/debug/timing/FlameGraph.hpp"

namespace fgl::engine
{

	Matrix< MatrixType::WorldToScreen > Camera::getProjectionViewMatrix() const
	{
		assert( m_projection_matrix != constants::MAT4_IDENTITY );
		return m_projection_matrix * m_view_matrix;
	}

	glm::mat4 Camera::getInverseViewMatrix() const
	{
		return glm::inverse( m_view_matrix );
	}

	void Camera::setOrthographicProjection( float left, float right, float top, float bottom, float near, float far )
	{
		m_projection_matrix = Matrix< MatrixType::CameraToScreen >( glm::ortho( left, right, bottom, top, near, far ) );

		//TODO: Figure out frustum culling for orthographic projection. (If we even wanna use it)
	}

	FGL_FLATTEN_HOT void Camera::
		setPerspectiveProjection( const float fovy, const float aspect, const float near, const float far )
	{
		m_projection_matrix = Matrix< MatrixType::CameraToScreen >( glm::perspective( fovy, aspect, near, far ) );

		m_base_frustum = createFrustum( aspect, fovy, near, far );
	}

	Coordinate< CoordinateSpace::World > Camera::getPosition() const
	{
		//Should maybe store the inverse view matrix
		return WorldCoordinate( m_inverse_view_matrix[ 3 ] );
	}

	void Camera::updateInfo( const FrameIndex frame_index )
	{
		ZoneScoped;
		CameraInfo current_camera_info { .projection = getProjectionMatrix(),
			                             .view = getViewMatrix(),
			                             .inverse_view = getInverseViewMatrix() };

		m_camera_frame_info[ frame_index ] = current_camera_info;
	}

	descriptors::DescriptorSet& Camera::getDescriptor( const FrameIndex index )
	{
		assert( index < m_camera_info_descriptors.size() );
		return *m_camera_info_descriptors[ index ];
	}

	void Camera::setFOV( const float fov_y )
	{
		m_fov_y = fov_y;
		setPerspectiveProjection( m_fov_y, aspectRatio(), constants::NEAR_PLANE, constants::FAR_PLANE );
	}

	void Camera::pass( FrameInfo& frame_info )
	{
		ZoneScopedN( "Camera::pass" );
		auto timer = debug::timing::push( "Camera" );
		if ( m_cold && m_gbuffer_swapchain )
		{
			//TODO: Make some way to destroy the swapchain in a deffered manner.
			// m_old_swapchain = m_gbuffer_swapchain;
			m_gbuffer_swapchain = nullptr;
			m_active = false;
		}

		if ( !m_active ) return;

		assert( frame_info.camera == nullptr );
		frame_info.camera = this;

		if ( m_gbuffer_swapchain->getExtent() != m_target_extent )
		{
			remakeSwapchain( m_target_extent );
		}

		updateInfo( frame_info.in_flight_idx );
		FGL_ASSERT( m_camera_renderer, "Camera renderer should not be nullptr" );
		m_camera_renderer->pass( frame_info, *m_gbuffer_swapchain );
		frame_info.camera = nullptr;
	}

	GBufferSwapchain& Camera::getSwapchain() const
	{
		return *m_gbuffer_swapchain;
	}

	CompositeSwapchain& Camera::getCompositeSwapchain() const
	{
		return *m_composite_swapchain;
	}

	void Camera::setViewport( const vk::raii::CommandBuffer& command_buffer )
	{
		vk::Viewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;

		const auto& [ width, height ] = m_gbuffer_swapchain->getExtent();
		viewport.width = static_cast< float >( width );
		viewport.height = static_cast< float >( height );
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		const std::vector< vk::Viewport > viewports { viewport };

		command_buffer.setViewport( 0, viewports );
	}

	void Camera::setScissor( const vk::raii::CommandBuffer& command_buffer )
	{
		const vk::Rect2D scissor { { 0, 0 }, m_gbuffer_swapchain->getExtent() };

		const std::vector< vk::Rect2D > scissors { scissor };

		command_buffer.setScissor( 0, scissors );
	}

	void Camera::remakeSwapchain( vk::Extent2D extent )
	{
		this->setPerspectiveProjection( m_fov_y, aspectRatio(), constants::NEAR_PLANE, constants::FAR_PLANE );

		log::debug( "Camera swapchain recreated" );

		m_old_composite_swapchain.push( std::move( m_composite_swapchain ) );
		m_old_gbuffer_swapchain.push( std::move( m_gbuffer_swapchain ) );

		if ( m_old_composite_swapchain.size() > constants::MAX_FRAMES_IN_FLIGHT ) m_old_composite_swapchain.pop();
		if ( m_old_gbuffer_swapchain.size() > constants::MAX_FRAMES_IN_FLIGHT ) m_old_gbuffer_swapchain.pop();

		m_composite_swapchain = std::make_unique< CompositeSwapchain >( extent );
		m_gbuffer_swapchain = std::make_unique< GBufferSwapchain >( extent );
	}

	void Camera::setName( const std::string_view str )
	{
		m_name = str;
	}

	float Camera::aspectRatio() const
	{
		return m_gbuffer_swapchain->getAspectRatio();
	}

	void Camera::
		copyOutput( const vk::raii::CommandBuffer& command_buffer, const FrameIndex frame_index, Image& target )
	{
		assert( m_gbuffer_swapchain->getExtent() == target.getExtent() );

		Texture& source_tex { *m_composite_swapchain->m_gbuffer_target[ frame_index ] };
		Image& source { source_tex.getImageRef() };

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
		region.extent = vk::Extent3D( m_gbuffer_swapchain->getExtent(), 1 );

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

	void Camera::updateMatrix()
	{
		const auto& [ pos, scale, rotation ] = m_transform;

		const auto rotation_matrix { rotation.forcedQuat().mat() };

		const glm::vec3 forward { rotation_matrix * glm::vec4( constants::WORLD_FORWARD, 0.0f ) };

		const glm::vec3 camera_up { rotation_matrix * glm::vec4( -constants::WORLD_Z, 0.0f ) };

		const WorldCoordinate center_pos { pos + forward };

		m_view_matrix = Matrix< MatrixType::WorldToCamera >( glm::lookAt( pos.vec(), center_pos.vec(), camera_up ) );

		m_inverse_view_matrix = glm::inverse( m_view_matrix );

		updateFrustum();
	}

	FGL_FLATTEN_HOT void Camera::setView( const WorldCoordinate pos, const QuatRotation& rotation, const ViewMode mode )
	{
		switch ( mode )
		{
			case ViewMode::TaitBryan:
				{
					m_transform.translation = pos;
					m_transform.rotation = rotation;
					updateMatrix();
					break;
				}
			case ViewMode::Euler:
				[[fallthrough]];
				{
					//TODO: Implement
					//view_matrix = glm::lookAt(position, position + );
				}
			default:
				throw std::runtime_error( "Unimplemented view mode" );
		}

		updateFrustum();
	}

	void Camera::updateFrustum()
	{
		m_last_frustum_pos = getPosition();

		const Matrix< MatrixType::ModelToWorld > translation_matrix { frustumTranslationMatrix() };

		m_frustum = translation_matrix * m_base_frustum;
	}

	const std::string& Camera::getName() const
	{
		return m_name;
	}

	constexpr descriptors::Descriptor camera_descriptor { 0,
		                                                  vk::DescriptorType::eUniformBuffer,
		                                                  vk::ShaderStageFlagBits::eAllGraphics };

	inline static descriptors::DescriptorSetLayout camera_descriptor_set { 1, camera_descriptor };

	descriptors::DescriptorSetLayout& Camera::getDescriptorLayout()
	{
		return camera_descriptor_set;
	}

	Camera::Camera(
		const vk::Extent2D extent, memory::Buffer& buffer, const std::shared_ptr< GBufferRenderer >& renderer ) :
	  m_target_extent( extent ),
	  m_composite_swapchain( std::make_unique< CompositeSwapchain >( m_target_extent ) ),
	  m_gbuffer_swapchain( std::make_unique< GBufferSwapchain >( m_target_extent ) ),
	  m_camera_renderer( renderer ),
	  m_camera_frame_info( buffer, constants::MAX_FRAMES_IN_FLIGHT )
	{
		FGL_ASSERT( renderer, "Camera renderer is null" );
		this->setPerspectiveProjection( m_fov_y, aspectRatio(), constants::NEAR_PLANE, constants::FAR_PLANE );
		this->setView( WorldCoordinate( constants::CENTER ), QuatRotation( 0.0f, 0.0f, 0.0f ) );

		for ( std::uint8_t i = 0; i < constants::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			auto set { camera_descriptor_set.create() };
			set->bindUniformBuffer( 0, m_camera_frame_info[ i ] );
			set->update();
			set->setName( std::format( "Camera {} descriptor set {}", m_camera_idx, i ) );

			m_camera_info_descriptors.emplace_back( std::move( set ) );
		}
	}

	void Camera::setExtent( const vk::Extent2D extent )
	{
		m_target_extent = extent;
	}

	FrustumBase createFrustum( const float aspect, const float fov_y, const float near, const float far )
	{
		const Plane< CoordinateSpace::Model > near_plane { ModelCoordinate( constants::WORLD_FORWARD * near ),
			                                               NormalVector( constants::WORLD_FORWARD ) };
		const Plane< CoordinateSpace::Model > far_plane { ModelCoordinate( constants::WORLD_FORWARD * far ),
			                                              NormalVector( -constants::WORLD_FORWARD ) };

		const float half_height { far * glm::tan( fov_y / 2.0f ) };
		const float half_width { half_height * aspect };

		const ModelCoordinate far_forward { constants::WORLD_FORWARD * far };
		const ModelCoordinate right_half { constants::WORLD_RIGHT * half_width };

		const Vector right_forward { ( far_forward + right_half ).vec() };
		const Vector left_forward { ( far_forward - right_half ).vec() };

		const Plane< CoordinateSpace::Model > right_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( right_forward.vec(), constants::WORLD_Z_NEG ) )
		};
		const Plane< CoordinateSpace::Model > left_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( left_forward.vec(), constants::WORLD_Z ) )
		};

		const ModelCoordinate top_half { constants::WORLD_Z * half_height };

		const Vector top_forward { ( far_forward + top_half ).vec() };
		const Vector bottom_forward { ( far_forward - top_half ).vec() };

		const Plane< CoordinateSpace::Model > top_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( top_forward.vec(), constants::WORLD_RIGHT ) )
		};

		const Plane< CoordinateSpace::Model > bottom_plane {
			ModelCoordinate( constants::WORLD_CENTER ),
			NormalVector( glm::cross( bottom_forward.vec(), -constants::WORLD_RIGHT ) )
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
		return m_transform.mat();
	}

	WorldCoordinate Camera::getFrustumPosition() const
	{
		return m_last_frustum_pos;
	}

	Camera::~Camera()
	{}

	CameraIDX Camera::getIDX() const
	{
		return m_camera_idx;
	}

} // namespace fgl::engine
