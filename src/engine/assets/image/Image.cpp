//
// Created by kj16609 on 12/4/23.
//

#include "Image.hpp"

#include "ImageView.hpp"
#include "rendering/ResourceWatcher.hpp"

namespace fgl::engine
{

	std::shared_ptr< ImageView > Image::getView( Sampler sampler )
	{
		if ( !m_view.expired() ) return m_view.lock();

		assert( m_handle );
		auto ptr { std::make_shared< ImageView >( m_handle, std::move( sampler ) ) };
		m_view = ptr;
		return ptr;
	}

	constexpr vk::AccessFlags getAccessFlags( const vk::ImageLayout layout )
	{
		switch ( layout )
		{
			default:
				FGL_UNREACHABLE();
			case vk::ImageLayout::eUndefined:
				return vk::AccessFlagBits::eNone;
			case vk::ImageLayout::eGeneral:
				break;
			case vk::ImageLayout::eColorAttachmentOptimal:
				return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
				break;
			case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
				break;
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
			case vk::ImageLayout::eTransferSrcOptimal:
				return vk::AccessFlagBits::eTransferRead;
			case vk::ImageLayout::eTransferDstOptimal:
				return vk::AccessFlagBits::eTransferWrite;
			case vk::ImageLayout::ePreinitialized:
				return vk::AccessFlagBits::eHostWrite;
			case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:
				break;
			case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:
				break;
			case vk::ImageLayout::eStencilAttachmentOptimal:
				break;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				[[fallthrough]];
			case vk::ImageLayout::eDepthReadOnlyOptimal:
				[[fallthrough]];
			case vk::ImageLayout::eDepthAttachmentOptimal:
				return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			case vk::ImageLayout::eStencilReadOnlyOptimal:
				break;
			case vk::ImageLayout::eReadOnlyOptimal:
				break;
			case vk::ImageLayout::eAttachmentOptimal:
				break;
			case vk::ImageLayout::ePresentSrcKHR:
				return vk::AccessFlags( 0 );
			case vk::ImageLayout::eVideoDecodeDstKHR:
				break;
			case vk::ImageLayout::eVideoDecodeSrcKHR:
				break;
			case vk::ImageLayout::eVideoDecodeDpbKHR:
				break;
			case vk::ImageLayout::eSharedPresentKHR:
				break;
			case vk::ImageLayout::eFragmentDensityMapOptimalEXT:
				break;
			case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
				return vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR;
			case vk::ImageLayout::eRenderingLocalReadKHR:
				return vk::AccessFlagBits::eColorAttachmentWrite;
			case vk::ImageLayout::eVideoEncodeDstKHR:
				break;
			case vk::ImageLayout::eVideoEncodeSrcKHR:
				break;
			case vk::ImageLayout::eVideoEncodeDpbKHR:
				break;
			case vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT:
				break;
			case vk::ImageLayout::eVideoEncodeQuantizationMapKHR:
				break;
		}

		FGL_UNREACHABLE();
	}

	constexpr vk::PipelineStageFlags getPipelineStageFlags( const vk::ImageLayout layout )
	{
		switch ( layout )
		{
			default:
				FGL_UNREACHABLE();
			case vk::ImageLayout::eUndefined:
				return vk::PipelineStageFlagBits::eTopOfPipe;
			case vk::ImageLayout::eGeneral:
				break;
			case vk::ImageLayout::eColorAttachmentOptimal:
				return vk::PipelineStageFlagBits::eColorAttachmentOutput;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				break;
			case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
				break;
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				return vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
			case vk::ImageLayout::eTransferSrcOptimal:
				[[fallthrough]];
			case vk::ImageLayout::eTransferDstOptimal:
				return vk::PipelineStageFlagBits::eTransfer;
			case vk::ImageLayout::ePreinitialized:
				break;
			case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:
				break;
			case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:
				break;
			case vk::ImageLayout::eDepthAttachmentOptimal:
				break;
			case vk::ImageLayout::eDepthReadOnlyOptimal:
				break;
			case vk::ImageLayout::eStencilAttachmentOptimal:
				break;
			case vk::ImageLayout::eStencilReadOnlyOptimal:
				break;
			case vk::ImageLayout::eReadOnlyOptimal:
				break;
			case vk::ImageLayout::eAttachmentOptimal:
				break;
			case vk::ImageLayout::ePresentSrcKHR:
				return vk::PipelineStageFlagBits::eBottomOfPipe;
			case vk::ImageLayout::eVideoDecodeDstKHR:
				break;
			case vk::ImageLayout::eVideoDecodeSrcKHR:
				break;
			case vk::ImageLayout::eVideoDecodeDpbKHR:
				break;
			case vk::ImageLayout::eSharedPresentKHR:
				break;
			case vk::ImageLayout::eFragmentDensityMapOptimalEXT:
				break;
			case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
				break;
			case vk::ImageLayout::eRenderingLocalReadKHR:
				break;
			case vk::ImageLayout::eVideoEncodeDstKHR:
				break;
			case vk::ImageLayout::eVideoEncodeSrcKHR:
				break;
			case vk::ImageLayout::eVideoEncodeDpbKHR:
				break;
			case vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT:
				break;
			case vk::ImageLayout::eVideoEncodeQuantizationMapKHR:
				break;
		}

		FGL_UNREACHABLE();
	}

	vk::ImageMemoryBarrier Image::transitionTo(
		const vk::ImageLayout old_layout,
		const vk::ImageLayout new_layout,
		const vk::ImageSubresourceRange& range ) const
	{
		vk::ImageMemoryBarrier barrier {};
		barrier.srcAccessMask = getAccessFlags( old_layout );
		barrier.dstAccessMask = getAccessFlags( new_layout );

		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.subresourceRange = range;

		barrier.setImage( this->getVkImage() );

		return barrier;
	}

	vk::ImageMemoryBarrier Image::transitionTo(
		const vk::ImageLayout old_layout, const vk::ImageLayout new_layout, const vk::ImageAspectFlags aspect )
	{
		assert( m_handle->m_name.empty() == false && "Image name not assigned" );
		const vk::ImageSubresourceRange subresource { aspect, 0, 1, 0, 1 };

		const vk::ImageMemoryBarrier barrier { transitionTo( old_layout, new_layout, subresource ) };

		watcher::registerUse( this->shared_from_this() );

		return barrier;
	}

	Image::Image(
		const vk::Extent2D extent,
		const vk::Format format,
		const vk::Image image,
		const vk::ImageUsageFlags usage ) noexcept :
	  m_handle( std::make_shared< ImageHandle >( extent, format, image, usage ) ),
	  m_extent( extent )
	{}

	Image::Image(
		const vk::Extent2D extent,
		const vk::Format format,
		const vk::ImageUsageFlags usage,
		const vk::ImageLayout inital_layout,
		const vk::ImageLayout final_layout ) :
	  m_handle( std::make_shared< ImageHandle >( extent, format, usage, inital_layout, final_layout ) ),
	  m_extent( extent )
	{}

	Image& Image::operator=( const Image& other )
	{
		m_handle = other.m_handle;
		m_view = {};
		return *this;
	}

	VkImage Image::getVkImage() const
	{
		return m_handle->getVkImage();
	}

	Image& Image::operator=( Image&& other ) noexcept
	{
		m_handle = std::move( other.m_handle );
		m_view = std::move( other.m_view );
		m_extent = other.m_extent;
		return *this;
	}

	Image& Image::setName( const std::string& str )
	{
		m_handle->setName( str );
		return *this;
	}

} // namespace fgl::engine