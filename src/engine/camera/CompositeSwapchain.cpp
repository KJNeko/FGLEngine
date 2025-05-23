//
// Created by kj16609 on 1/13/25.
//
#include "CompositeSwapchain.hpp"

#include "engine/rendering/PresentSwapChain.hpp"
#include "engine/rendering/pipelines/Attachment.hpp"

namespace fgl::engine
{
	class Texture;

	void CompositeSwapchain::transitionImages( CommandBuffer& command_buffer, StageID stage_id, FrameIndex index )
	{
		switch ( stage_id )
		{
			default:
				throw std::runtime_error( "Invalid StageID" );
			case INITAL:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_buffer.m_target.getImage( index )
							.transitionColorTo( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal ),
					};

					command_buffer->pipelineBarrier(
						vk::PipelineStageFlagBits::eTopOfPipe,
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
			case FINAL:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_buffer.m_target.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal ),
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
			case FINAL_PRESENT:
				{
					const std::vector< vk::ImageMemoryBarrier > barriers {
						m_buffer.m_target.getImage( index ).transitionColorTo(
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR ),
					};

					command_buffer->pipelineBarrier(
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eBottomOfPipe,
						vk::DependencyFlags( 0 ),
						{},
						{},
						barriers );

					return;
				}
		}

		FGL_UNREACHABLE();
	}

	vk::RenderingInfo CompositeSwapchain::getRenderingInfo( const FrameIndex index )
	{
		static thread_local std::vector< vk::RenderingAttachmentInfo > infos {};

		infos.clear();

		infos = { m_buffer.m_target.renderInfo( index, vk::ImageLayout::eColorAttachmentOptimal ) };

		vk::RenderingInfo rendering_info {};

		rendering_info.setRenderArea( { { 0, 0 }, m_extent } );

		rendering_info.setLayerCount( 1 );

		rendering_info.setColorAttachments( infos );

		return rendering_info;
	}

	CompositeSwapchain::CompositeSwapchain( vk::Extent2D extent ) : m_extent( extent )
	{
		constexpr auto image_count { constants::MAX_FRAMES_IN_FLIGHT };

		m_buffer.m_target.createResources( image_count, m_extent, vk::ImageUsageFlagBits::eTransferSrc );
		m_buffer.m_target.setName( "CompositeSwapchain::m_target" );

		for ( const auto& image : m_buffer.m_target.m_attachment_resources.m_images )
		{
			Sampler default_sampler {};
			m_gbuffer_target.emplace_back( std::make_unique< Texture >( image, std::move( default_sampler ) ) );
		}
	}

	CompositeSwapchain::~CompositeSwapchain()
	{}

} // namespace fgl::engine
