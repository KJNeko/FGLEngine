//
// Created by kj16609 on 7/11/24.
//

#include "CameraSwapchain.hpp"

#include "engine/descriptors/DescriptorSet.hpp"

namespace fgl::engine
{

	std::vector< std::unique_ptr< descriptors::DescriptorSet > > CameraSwapchain::createGBufferDescriptors()
	{
		std::vector< std::unique_ptr< descriptors::DescriptorSet > > data {};
		data.resize( SwapChain::MAX_FRAMES_IN_FLIGHT );

		for ( PresentIndex i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			//auto set { std::make_unique< descriptors::DescriptorSet >( GBufferDescriptorSet::createLayout() ) };
			auto set { gbuffer_set.create() };

			set->bindAttachment( 0, m_gbuffer.m_color.getView( i ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->bindAttachment( 1, m_gbuffer.m_position.getView( i ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->bindAttachment( 2, m_gbuffer.m_normal.getView( i ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->bindAttachment( 3, m_gbuffer.m_metallic.getView( i ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->bindAttachment( 4, m_gbuffer.m_emissive.getView( i ), vk::ImageLayout::eShaderReadOnlyOptimal );

			set->update();

			data[ i ] = std::move( set );
		}

		return data;
	}

	vk::ImageMemoryBarrier createColorImageBarrier(
		const Image& image,
		const vk::ImageLayout old_layout,
		const vk::ImageLayout new_layout,
		const vk::AccessFlags flags )
	{
		vk::ImageMemoryBarrier barrier {};

		barrier.setImage( image.getVkImage() );
		barrier.setOldLayout( old_layout );
		barrier.setNewLayout( new_layout );
		barrier.setSrcAccessMask( flags );

		constexpr vk::ImageSubresourceRange subresource { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

		barrier.setSubresourceRange( subresource );

		return barrier;
	}

	vk::ImageMemoryBarrier createDepthImageBarrier(
		const Image& image, const vk::ImageLayout old_layout, const vk::ImageLayout new_layout )
	{
		vk::ImageMemoryBarrier barrier {};
		barrier.setImage( image.getVkImage() );
		barrier.setOldLayout( old_layout );
		barrier.setNewLayout( new_layout );
		constexpr vk::ImageSubresourceRange subresource {
			vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1,
		};

		barrier.setSubresourceRange( subresource );
		return barrier;
	}

	void CameraSwapchain::transitionImages(
		vk::raii::CommandBuffer& command_buffer, const std::uint16_t stage_id, const FrameIndex index )
	{
		switch ( stage_id )
		{
			default:
				throw std::invalid_argument( "Invalid Stage ID" );
			case INITAL:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						createColorImageBarrier(
							m_gbuffer.m_color.getImage( index ),
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::eColorAttachmentOptimal,
							vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead ),
						createColorImageBarrier(
							m_gbuffer.m_emissive.getImage( index ),
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::eColorAttachmentOptimal,
							vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead ),
						createColorImageBarrier(
							m_gbuffer.m_metallic.getImage( index ),
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::eColorAttachmentOptimal,
							vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead ),
						createColorImageBarrier(
							m_gbuffer.m_position.getImage( index ),
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::eColorAttachmentOptimal,
							vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead ),
						createDepthImageBarrier(
							m_gbuffer.m_depth.getImage( index ),
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::eDepthStencilAttachmentOptimal )
					};

					command_buffer.pipelineBarrier(
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eBottomOfPipe,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
			case FINAL:
				{
					return;
				}
		}
	}

	vk::RenderingInfo CameraSwapchain::getRenderingInfo( const FrameIndex frame_index )
	{
		// This should be safe to have as static as the information used here will only capable of being used in a single frame.
		static thread_local std::vector< vk::RenderingAttachmentInfo > color_attachment_infos {};
		static thread_local vk::RenderingAttachmentInfo depth_attachment_infos {};

		depth_attachment_infos = m_gbuffer.m_depth.renderInfo( frame_index, vk::ImageLayout::eDepthAttachmentOptimal );

		color_attachment_infos.clear();
		color_attachment_infos = {
			m_gbuffer.m_color.renderInfo( frame_index, vk::ImageLayout::eColorAttachmentOptimal ),
			m_gbuffer.m_position.renderInfo( frame_index, vk::ImageLayout::eColorAttachmentOptimal ),
			m_gbuffer.m_normal.renderInfo( frame_index, vk::ImageLayout::eColorAttachmentOptimal ),
			m_gbuffer.m_metallic.renderInfo( frame_index, vk::ImageLayout::eColorAttachmentOptimal ),
			m_gbuffer.m_emissive.renderInfo( frame_index, vk::ImageLayout::eColorAttachmentOptimal )
		};

		vk::RenderingInfo rendering_info {};

		rendering_info.setRenderArea( { { 0, 0 }, m_extent } );

		rendering_info.setLayerCount( 1 );

		rendering_info.setColorAttachments( color_attachment_infos );
		rendering_info.setPDepthAttachment( &depth_attachment_infos );
		// rendering_info.setPStencilAttachment( &depth_attachment_infos );

		return rendering_info;
	}

	const std::vector< vk::ClearValue >& CameraSwapchain::getClearValues()
	{
		assert( !m_clear_values.empty() );
		return m_clear_values;
	}

	std::vector< vk::raii::Framebuffer > CameraSwapchain::createFrambuffers()
	{
		constexpr auto image_count { SwapChain::MAX_FRAMES_IN_FLIGHT };

		m_gbuffer.m_color.createResources( image_count, m_extent );
		m_gbuffer.m_position.createResources( image_count, m_extent );
		m_gbuffer.m_normal.createResources( image_count, m_extent );
		m_gbuffer.m_metallic.createResources( image_count, m_extent );
		m_gbuffer.m_emissive.createResources( image_count, m_extent );

		m_gbuffer.m_composite.createResources( image_count, m_extent, vk::ImageUsageFlagBits::eTransferSrc );
		m_gbuffer.m_depth.createResources( image_count, m_extent );
		m_gbuffer.m_depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		std::vector< vk::raii::Framebuffer > buffers {};
		buffers.reserve( image_count );

		for ( FrameIndex i = 0; i < image_count; ++i )
		{
			std::vector< vk::ImageView > attachments { getViewsForFrame(
				i,
				m_gbuffer.m_color,
				m_gbuffer.m_position,
				m_gbuffer.m_normal,
				m_gbuffer.m_metallic,
				m_gbuffer.m_emissive,
				m_gbuffer.m_composite,
				m_gbuffer.m_depth ) };

			vk::FramebufferCreateInfo info {};
			info.renderPass = m_renderpass;
			info.setAttachments( attachments );
			info.width = m_extent.width;
			info.height = m_extent.height;
			info.layers = 1;

			buffers.emplace_back( Device::getInstance()->createFramebuffer( info ) );

			m_g_buffer_color_img.emplace_back(
				std::make_unique< Texture >( m_gbuffer.m_color.m_attachment_resources.m_images[ i ]
			                                     ->setName( "GBufferColor" ) ) );

			auto& position_resources { m_gbuffer.m_position.m_attachment_resources };
			assert( position_resources.m_images[ i ] );
			assert( position_resources.m_image_views[ i ] );
			auto& position_image { *position_resources.m_images[ i ] };
			position_image.setName( format_ns::format( "GBufferPosition: {}", i ) );
			m_g_buffer_position_img.emplace_back( std::make_unique< Texture >( position_image ) );

			m_g_buffer_normal_img.emplace_back(
				std::make_unique< Texture >( m_gbuffer.m_normal.m_attachment_resources.m_images[ i ]
			                                     ->setName( "GBufferNormal" ) ) );

			m_g_buffer_metallic_img.emplace_back(
				std::make_unique< Texture >( m_gbuffer.m_metallic.m_attachment_resources.m_images[ i ]
			                                     ->setName( "GBufferMetallic" ) ) );

			m_g_buffer_emissive_img.emplace_back(
				std::make_unique< Texture >( m_gbuffer.m_emissive.m_attachment_resources.m_images[ i ]
			                                     ->setName( "GBufferEmissive" ) ) );

			m_g_buffer_composite_img.emplace_back(
				std::make_unique< Texture >( m_gbuffer.m_composite.m_attachment_resources.m_images[ i ]
			                                     ->setName( "GBufferComposite" ) ) );
		}

		return buffers;
	}

	descriptors::DescriptorSet& CameraSwapchain::getGBufferDescriptor( const FrameIndex frame_index )
	{
		return *m_gbuffer_descriptor_set[ frame_index ];
	}

	vk::raii::Framebuffer& CameraSwapchain::getFramebuffer( const FrameIndex frame_index )
	{
		return m_framebuffers[ frame_index ];
	}

	vk::Extent2D CameraSwapchain::getExtent() const
	{
		return m_extent;
	}

	Image& CameraSwapchain::getOutput( const FrameIndex index )
	{
		assert( index <= this->m_gbuffer.m_composite.m_attachment_resources.m_images.size() );
		return *m_gbuffer.m_composite.m_attachment_resources.m_images[ index ];
	}

	float CameraSwapchain::getAspectRatio()
	{
		return static_cast< float >( m_extent.width ) / static_cast< float >( m_extent.height );
	}

	CameraSwapchain::CameraSwapchain( vk::raii::RenderPass& renderpass, const vk::Extent2D extent ) :
	  m_extent( extent ),
	  m_renderpass( renderpass ),
	  m_framebuffers( createFrambuffers() ),
	  m_clear_values( gatherClearValues(
		  m_gbuffer.m_color,
		  m_gbuffer.m_position,
		  m_gbuffer.m_normal,
		  m_gbuffer.m_metallic,
		  m_gbuffer.m_emissive,
		  m_gbuffer.m_composite,
		  m_gbuffer.m_depth ) ),
	  m_gbuffer_descriptor_set( createGBufferDescriptors() )
	{
		m_gbuffer.m_depth.setName( "Depth" );
	}

	CameraSwapchain::~CameraSwapchain()
	{
		for ( auto& descriptor : m_gbuffer_descriptor_set )
		{
			descriptors::queueDescriptorDeletion( std::move( descriptor ) );
		}
	}

} // namespace fgl::engine