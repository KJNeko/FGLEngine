//
// Created by kj16609 on 7/11/24.
//

#include "GBufferSwapchain.hpp"

#include "engine/descriptors/DescriptorSet.hpp"

namespace fgl::engine
{

	std::vector< std::unique_ptr< descriptors::DescriptorSet > > GBufferSwapchain::createGBufferDescriptors()
	{
		std::vector< std::unique_ptr< descriptors::DescriptorSet > > data {};
		data.resize( constants::MAX_FRAMES_IN_FLIGHT );

		for ( PresentIndex i = 0; i < constants::MAX_FRAMES_IN_FLIGHT; ++i )
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

	void GBufferSwapchain::
		transitionImages( CommandBuffer& command_buffer, const std::uint16_t stage_id, const FrameIndex index )
	{
		switch ( stage_id )
		{
			default:
				throw std::invalid_argument( "Invalid Stage ID" );
			case INITAL:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_gbuffer.m_color.getImage( index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
						m_gbuffer.m_emissive.getImage( index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
						m_gbuffer.m_metallic.getImage( index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
						m_gbuffer.m_position.getImage( index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
						m_gbuffer.m_normal.getImage( index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
						m_gbuffer.m_depth.getImage( index ).transitionTo(
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::eDepthAttachmentOptimal,
							vk::ImageAspectFlagBits::eDepth ),
					};

					command_buffer->pipelineBarrier(
						vk::PipelineStageFlagBits::eTopOfPipe,
						vk::PipelineStageFlagBits::eColorAttachmentOutput
							| vk::PipelineStageFlagBits::eEarlyFragmentTests
							| vk::PipelineStageFlagBits::eLateFragmentTests,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
			case FINAL:
				{
					// Final should turn all of these into readonly in order to be used for other render targets.
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_gbuffer.m_color.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal ),
						m_gbuffer.m_emissive.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal ),
						m_gbuffer.m_metallic.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal ),
						m_gbuffer.m_position.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal ),
						m_gbuffer.m_normal.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal )
						// It might be useful to properly transition this for other post processing effects later
						// m_gbuffer.m_depth.getImage( index ).transitionTo(
						// 	vk::ImageLayout::eDepthAttachmentOptimal,
						// 	vk::ImageLayout::eDepthReadOnlyOptimal,
						// 	vk::ImageAspectFlagBits::eDepth ),
					};

					command_buffer->pipelineBarrier(
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eFragmentShader,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
		}
	}

	vk::RenderingInfo GBufferSwapchain::getRenderingInfo( const FrameIndex frame_index )
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

	descriptors::DescriptorSet& GBufferSwapchain::getGBufferDescriptor( const FrameIndex frame_index )
	{
		return *m_gbuffer_descriptor_set[ frame_index ];
	}

	vk::Extent2D GBufferSwapchain::getExtent() const
	{
		return m_extent;
	}

	float GBufferSwapchain::getAspectRatio()
	{
		return static_cast< float >( m_extent.width ) / static_cast< float >( m_extent.height );
	}

	GBufferSwapchain::GBufferSwapchain( const vk::Extent2D extent ) : m_extent( extent )
	// m_gbuffer_descriptor_set( createGBufferDescriptors() )
	{
		constexpr auto image_count { constants::MAX_FRAMES_IN_FLIGHT };

		m_gbuffer.m_color.createResources( image_count, m_extent );
		m_gbuffer.m_position.createResources( image_count, m_extent );
		m_gbuffer.m_normal.createResources( image_count, m_extent );
		m_gbuffer.m_metallic.createResources( image_count, m_extent );
		m_gbuffer.m_emissive.createResources( image_count, m_extent );

		m_gbuffer.m_depth.createResources( image_count, m_extent );
		m_gbuffer.m_depth.setClear( vk::ClearDepthStencilValue( 1.0f, 0 ) );

		m_gbuffer.m_color.setName( "Color" );
		m_gbuffer.m_position.setName( "Position" );
		m_gbuffer.m_normal.setName( "Normal" );
		m_gbuffer.m_metallic.setName( "Metallic" );
		m_gbuffer.m_emissive.setName( "Emissive" );
		m_gbuffer.m_depth.setName( "Depth" );

		m_gbuffer_descriptor_set = createGBufferDescriptors();
	}

	GBufferSwapchain::~GBufferSwapchain()
	{
		for ( auto& descriptor : m_gbuffer_descriptor_set )
		{
			descriptors::queueDescriptorDeletion( std::move( descriptor ) );
		}
	}

} // namespace fgl::engine