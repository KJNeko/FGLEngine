//
// Created by kj16609 on 7/2/24.
//

#include "TransferData.hpp"

#include "engine/assets/image/ImageHandle.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/memory/buffers/BufferHandle.hpp"
#include "engine/memory/buffers/exceptions.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"
#include "engine/utils.hpp"

namespace fgl::engine::memory
{

	std::size_t BufferHasher::operator()( const vk::Buffer& buffer ) const
	{
		return reinterpret_cast< std::size_t >( static_cast< VkBuffer >( buffer ) );
	}

	std::size_t CopyRegionKeyHasher::operator()( const std::pair< vk::Buffer, vk::Buffer >& pair ) const
	{
		const std::size_t hash_a { BufferHasher {}( std::get< 0 >( pair ) ) };
		const std::size_t hash_b { BufferHasher {}( std::get< 1 >( pair ) ) };

		std::size_t seed { 0 };
		hashCombine( seed, hash_a, hash_b );
		return seed;
	}

	bool TransferData::
		performImageStage( vk::raii::CommandBuffer& cmd_buffer, std::uint32_t transfer_idx, std::uint32_t graphics_idx )
	{
		auto& source_buffer { std::get< TransferBufferHandle >( m_source ) };
		auto& dest_image { std::get< TransferImageHandle >( m_target ) };

		vk::ImageSubresourceRange range;
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier barrier {};
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.image = dest_image->getVkImage();
		barrier.subresourceRange = range;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		const std::vector< vk::ImageMemoryBarrier > barriers_to { barrier };

		cmd_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			{},
			barriers_to );

		vk::BufferImageCopy region {};
		region.bufferOffset = source_buffer->getOffset();
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = vk::Offset3D( 0, 0, 0 );
		region.imageExtent = vk::Extent3D( dest_image->extent(), 1 );

		std::vector< vk::BufferImageCopy > regions { region };

		cmd_buffer.copyBufferToImage(
			source_buffer->getVkBuffer(), dest_image->getVkImage(), vk::ImageLayout::eTransferDstOptimal, regions );

		//Transfer back to eGeneral

		vk::ImageMemoryBarrier barrier_from {};
		barrier_from.oldLayout = barrier.newLayout;
		barrier_from.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier_from.image = dest_image->getVkImage();
		barrier_from.subresourceRange = range;
		barrier_from.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier_from.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier_from.srcQueueFamilyIndex = transfer_idx;
		barrier_from.dstQueueFamilyIndex = graphics_idx;

		const std::vector< vk::ImageMemoryBarrier > barriers_from { barrier_from };

		cmd_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			{},
			{},
			barriers_from );

		return true;
	}

	bool TransferData::performRawImageStage(
		vk::raii::CommandBuffer& buffer,
		Buffer& staging_buffer,
		const std::uint32_t transfer_idx,
		const std::uint32_t graphics_idx )
	{
		if ( !convertRawToBuffer( staging_buffer ) ) return false;
		return performImageStage( buffer, transfer_idx, graphics_idx );
	}

	bool TransferData::performBufferStage( CopyRegionMap& copy_regions )
	{
		ZoneScoped;
		auto& source { std::get< TransferBufferHandle >( m_source ) };
		auto& target { std::get< TransferBufferHandle >( m_target ) };
		const CopyRegionKey key { std::make_pair( source->getBuffer(), target->getBuffer() ) };

		const auto copy_info { source->copyRegion( *target, m_target_offset ) };

		if ( auto itter = copy_regions.find( key ); itter != copy_regions.end() )
		{
			auto& [ key_i, regions ] = *itter;
			regions.emplace_back( copy_info );
		}
		else
		{
			std::vector< vk::BufferCopy > copies { copy_info };
			copy_regions.insert( std::make_pair( key, copies ) );
		}

		return true;
	}

	bool TransferData::performRawBufferStage( Buffer& staging_buffer, CopyRegionMap& copy_regions )
	{
		if ( !convertRawToBuffer( staging_buffer ) ) return false;
		return performBufferStage( copy_regions );
	}

	bool TransferData::convertRawToBuffer( Buffer& staging_buffer )
	{
		// Prepare the staging buffer first.
		assert( std::holds_alternative< RawData >( m_source ) );
		assert( !std::get< RawData >( m_source ).empty() );

		// Check if we are capable of allocating into the staging buffer
		if ( !staging_buffer->canAllocate( std::get< RawData >( m_source ).size(), 1 ) ) return false;

		HostVector< std::byte > vector { staging_buffer, std::get< RawData >( m_source ) };

		m_source = vector.getHandle();
		std::get< TransferBufferHandle >( m_source )->setReady( true );

		return true;
	}

	bool TransferData::stage(
		vk::raii::CommandBuffer& buffer,
		Buffer& staging_buffer,
		CopyRegionMap& copy_regions,
		const std::uint32_t transfer_idx,
		const std::uint32_t graphics_idx )
	{
		ZoneScoped;

		switch ( m_type )
		{
			default:
				throw std::runtime_error( "Invalid transfer type" );
			case eImageFromRaw:
				{
					return performRawImageStage( buffer, staging_buffer, transfer_idx, graphics_idx );
				}
			case eImageFromBuffer:
				{
					if ( !std::get< TransferBufferHandle >( m_source )->transferReady() ) return false;
					return performImageStage( buffer, transfer_idx, graphics_idx );
				}
			case eBufferFromRaw:
				{
					return performRawBufferStage( staging_buffer, copy_regions );
				}
			case eBufferFromBuffer:
				{
					if ( !std::get< TransferBufferHandle >( m_source )->transferReady() ) return false;
					return performBufferStage( copy_regions );
				}
		}

		FGL_UNREACHABLE();
	}

	void TransferData::markBad() const
	{
		switch ( m_type )
		{
			case eBufferFromRaw:
				[[fallthrough]];
			case eBufferFromBuffer:
				std::get< TransferBufferHandle >( m_target )->setReady( false );
				break;
			case eImageFromRaw:
				[[fallthrough]];
			case eImageFromBuffer:
				std::get< TransferImageHandle >( m_target )->setReady( false );
		}
	}

	void TransferData::markGood() const
	{
		switch ( m_type )
		{
			case eBufferFromRaw:
				[[fallthrough]];
			case eBufferFromBuffer:
				std::get< TransferBufferHandle >( m_target )->setReady( true );
				break;
			case eImageFromRaw:
				[[fallthrough]];
			case eImageFromBuffer:
				std::get< TransferImageHandle >( m_target )->setReady( true );
		}
	}

	// Constructors

	//! BUFFER_FROM_BUFFER
	TransferData::TransferData(
		const std::shared_ptr< BufferSuballocationHandle >& source,
		const std::shared_ptr< BufferSuballocationHandle >& target,
		const vk::DeviceSize size,
		const vk::DeviceSize dst_offset,
		const vk::DeviceSize src_offset ) :
	  m_type( eBufferFromBuffer ),
	  m_source( source ),
	  m_source_offset( src_offset ),
	  m_target( target ),
	  m_target_offset( dst_offset ),
	  m_size( size == 0 ? source->m_size : size )
	{
		FGL_ASSERT( m_size <= target->m_size, "Attempting to copy to beyond size of target" );
		markBad();
	}

	//! BUFFER_FROM_RAW
	TransferData::TransferData(
		std::vector< std::byte >&& source,
		const std::shared_ptr< BufferSuballocationHandle >& target,
		const vk::DeviceSize size,
		const vk::DeviceSize dst_offset,
		const vk::DeviceSize src_offset ) :
	  m_type( eBufferFromRaw ),
	  m_source( std::forward< std::vector< std::byte > >( source ) ),
	  m_source_offset( src_offset ),
	  m_target( target ),
	  m_target_offset( dst_offset ),
	  m_size( size == 0 ? source.size() : size )
	{
		FGL_ASSERT( m_size <= target->m_size, "Attempting to copy to beyond size of target" );
		markBad();
	}

	//! IMAGE_FROM_BUFFER
	TransferData::TransferData(
		const std::shared_ptr< BufferSuballocationHandle >& source,
		const std::shared_ptr< ImageHandle >& target,
		const vk::DeviceSize size,
		const vk::DeviceSize src_offset ) :
	  m_type( eImageFromBuffer ),
	  m_source( source ),
	  m_source_offset( src_offset ),
	  m_target( target ),
	  m_target_offset( 0 ),
	  m_size( size == 0 ? source->m_size : size )
	{
		markBad();
	}

	//! IMAGE_FROM_RAW
	TransferData::TransferData(
		std::vector< std::byte >&& source,
		const std::shared_ptr< ImageHandle >& target,
		const vk::DeviceSize size,
		const vk::DeviceSize src_offset ) :
	  m_type( eImageFromRaw ),
	  m_source( std::forward< std::vector< std::byte > >( source ) ),
	  m_source_offset( src_offset ),
	  m_target( target ),
	  m_target_offset( 0 ),
	  m_size( size == 0 ? source.size() : size )
	{
		assert( std::get< RawData >( m_source ).size() > 0 );
		markBad();
	}

} // namespace fgl::engine::memory
