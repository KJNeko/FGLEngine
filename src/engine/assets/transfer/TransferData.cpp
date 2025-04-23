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
#include "memory/DefferedCleanup.hpp"

namespace fgl::engine::memory
{

	std::size_t BufferHasher::operator()( const std::shared_ptr< BufferHandle >& buffer ) const
	{
		return reinterpret_cast< std::size_t >( static_cast< VkBuffer >( buffer->getVkBuffer() ) );
	}

	std::size_t CopyRegionKeyHasher::
		operator()( const std::pair< std::shared_ptr< BufferHandle >, std::shared_ptr< BufferHandle > >& pair ) const
	{
		const std::size_t hash_a { BufferHasher {}( std::get< 0 >( pair ) ) };
		const std::size_t hash_b { BufferHasher {}( std::get< 1 >( pair ) ) };

		std::size_t seed { 0 };
		hashCombine( seed, hash_a, hash_b );
		return seed;
	}

	bool TransferData::
		performImageStage( CommandBuffer& cmd_buffer, std::uint32_t transfer_idx, std::uint32_t graphics_idx )
	{
		FGL_ASSERT( std::holds_alternative< TransferSuballocationHandle >( m_source ), "Source not a suballocation!" );
		FGL_ASSERT( std::holds_alternative< TransferImageHandle >( m_target ), "Target not an image!" );

		auto& source_buffer { std::get< TransferSuballocationHandle >( m_source ) };
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

		cmd_buffer->pipelineBarrier(
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

		cmd_buffer->copyBufferToImage(
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

		cmd_buffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			{},
			{},
			barriers_from );

		return true;
	}

	bool TransferData::performRawImageStage(
		CommandBuffer& buffer,
		Buffer& staging_buffer,
		const std::uint32_t transfer_idx,
		const std::uint32_t graphics_idx )
	{
		if ( !convertRawToBuffer( staging_buffer ) ) return false;
		return performImageStage( buffer, transfer_idx, graphics_idx );
	}

	bool TransferData::performSuballocationStage( CopyRegionMap& copy_regions )
	{
		ZoneScoped;
		FGL_ASSERT( std::holds_alternative< TransferSuballocationHandle >( m_source ), "Source not suballocation!" );
		FGL_ASSERT( std::holds_alternative< TransferSuballocationHandle >( m_target ), "Target not suballocation!" );
		const auto& source { std::get< TransferSuballocationHandle >( m_source ) };
		const auto& target { std::get< TransferSuballocationHandle >( m_target ) };

		const CopyRegionKey key { std::make_pair( source->m_parent_buffer, target->m_parent_buffer ) };

		/*
		// attempt to see if the buffer that is our parent is already added to this region list, If so, then we should process that first.
		if ( const auto itter = std::ranges::find_if(
				 copy_regions,
				 []( const auto& itter ) -> bool
				 {
					 const auto& [ key, regions ] = itter;
					 const std::pair< std::shared_ptr< BufferHandle >, std::shared_ptr< BufferHandle > >& key_i = key;
					 const auto& [ source, target ] = key_i;

					 // if the source has an old handle, that means we've not finished copying everything to it. So we must instead wait for those to be completed before trying to copy from it
					 return source->m_old_handle.expired();
				 } );
		     itter != copy_regions.end() )
		{
			return false;
		}
		*/

		const auto copy_info { source->copyRegion( *target, m_target_offset, m_source_offset ) };

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

	vk::BufferMemoryBarrier TransferData::readSourceBarrier()
	{
		ZoneScoped;

		FGL_ASSERT( std::holds_alternative< TransferSuballocationHandle >( m_source ), "Source not a suballocation!" );

		vk::BufferMemoryBarrier barrier {};
		barrier.buffer = std::get< TransferSuballocationHandle >( m_source )->getVkBuffer();
		barrier.offset = std::get< TransferSuballocationHandle >( m_source )->m_offset;
		barrier.size = std::get< TransferSuballocationHandle >( m_source )->m_size;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		return barrier;
	}

	bool TransferData::targetIsHostVisible() const
	{
		return std::holds_alternative< TransferSuballocationHandle >( m_target )
		    && std::get< TransferSuballocationHandle >( m_target )->m_parent_buffer->isHostVisible();
	}

	bool TransferData::sourceIsHostVisible() const
	{
		return std::holds_alternative< TransferSuballocationHandle >( m_source )
		    && std::get< TransferSuballocationHandle >( m_source )->m_parent_buffer->isHostVisible();
	}

	bool TransferData::targetNeedsFlush() const
	{
		return targetIsHostVisible()
		    && std::get< TransferSuballocationHandle >( m_target )->m_parent_buffer->needsFlush();
	}

	bool TransferData::
		performRawSuballocationStage( CommandBuffer& cmd_buffer, Buffer& staging_buffer, CopyRegionMap& copy_regions )
	{
		if ( targetIsHostVisible() )
		{
			// Since the target is visible to the host, We can instead just copy directly to the buffer.
			FGL_ASSERT(
				std::holds_alternative< TransferSuballocationHandle >( m_target ),
				"Target was expected to be a suballocation!" );
			const auto& target { std::get< TransferSuballocationHandle >( m_target ) };
			FGL_ASSERT( std::holds_alternative< RawData >( m_source ), "Source was expected to be raw data!" );

			FGL_ASSERT(
				m_source_offset <= std::get< RawData >( m_source ).size(),
				"Source offset was out of bounds of source!" );

			FGL_ASSERT( target->m_ptr != nullptr, "Target buffer was not mapped!" );

			std::memcpy(
				static_cast< std::byte* >( target->m_ptr ) + m_target_offset,
				std::get< RawData >( m_source ).data() + m_source_offset,
				target->m_size );

			// if it's not coherent then we must flush it manually.
			//TODO: Move this to be done while we record the command buffer so we can batch them together easily.
			if ( targetNeedsFlush() ) target->flush();

			return true;
		}
		if ( !convertRawToBuffer( staging_buffer ) ) return false;
		return performSuballocationStage( copy_regions );
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

		return true;
	}

	bool TransferData::stage(
		CommandBuffer& buffer,
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
			case TransferType::IMAGE_FROM_RAW:
				return performRawImageStage( buffer, staging_buffer, transfer_idx, graphics_idx );
			case TransferType::IMAGE_FROM_SUBALLOCATION:
				return performImageStage( buffer, transfer_idx, graphics_idx );
			case TransferType::SUBALLOCATION_FROM_SUBALLOCATION:
				return performSuballocationStage( copy_regions );
			case TransferType::SUBALLOCATION_FROM_RAW:
				return performRawSuballocationStage( buffer, staging_buffer, copy_regions );
		}

		FGL_UNREACHABLE();
	}

	bool TransferData::isReady() const
	{
		switch ( m_type )
		{
			default:
			case TransferType::IMAGE_FROM_RAW:
				return true;
			case TransferType::IMAGE_FROM_SUBALLOCATION:
				[[fallthrough]];
			case TransferType::SUBALLOCATION_FROM_SUBALLOCATION:
				return std::get< TransferSuballocationHandle >( m_source )->ready();
			case TransferType::SUBALLOCATION_FROM_RAW:
				return true;
		}
	}

	void TransferData::markBad()
	{
		switch ( m_type )
		{
			default:
			case TransferType::IMAGE_FROM_RAW:
				[[fallthrough]];
			case TransferType::IMAGE_FROM_SUBALLOCATION:
				std::get< TransferImageHandle >( m_target )->setReady( false );
				return;
			case TransferType::SUBALLOCATION_FROM_RAW:
				[[fallthrough]];
			case TransferType::SUBALLOCATION_FROM_SUBALLOCATION:
				std::get< TransferSuballocationHandle >( m_target )->setReady( false );
				return;
		}
	}

	void TransferData::markGood()
	{
		switch ( m_type )
		{
			default:
				throw std::runtime_error( "Invalid transfer type" );
			case TransferType::IMAGE_FROM_RAW:
				[[fallthrough]];
			case TransferType::IMAGE_FROM_SUBALLOCATION:
				std::get< TransferImageHandle >( m_target )->setReady( true );
				return;
			case TransferType::SUBALLOCATION_FROM_RAW:
				[[fallthrough]];
			case TransferType::SUBALLOCATION_FROM_SUBALLOCATION:
				std::get< TransferSuballocationHandle >( m_target )->setReady( true );
				return;
		}
	}

	void TransferData::cleanupSource()
	{
		switch ( m_type )
		{
			default:
				throw std::runtime_error( "Invalid transfer type" );
			case TransferType::IMAGE_FROM_RAW:
				[[fallthrough]];
			case TransferType::SUBALLOCATION_FROM_RAW:
				return;
			case TransferType::IMAGE_FROM_SUBALLOCATION:
				[[fallthrough]];
			case TransferType::SUBALLOCATION_FROM_SUBALLOCATION:
				if ( std::holds_alternative< TransferSuballocationHandle >( m_source ) )
					deferredDelete< std::shared_ptr<
						BufferSuballocationHandle > >( std::get< TransferSuballocationHandle >( m_source ) );
				break;
		}
	}

	// Constructors

	//! BUFFER_FROM_BUFFER
	TransferData::TransferData(
		const std::shared_ptr< BufferSuballocationHandle >& source,
		const std::shared_ptr< BufferSuballocationHandle >& target,
		const vk::DeviceSize target_offset,
		const vk::DeviceSize source_offset ) :
	  m_type( TransferType::SUBALLOCATION_FROM_SUBALLOCATION ),
	  m_source( source ),
	  m_target( target ),
	  m_target_offset( target_offset ),
	  m_source_offset( source_offset )
	{
		markBad();
	}

	//! BUFFER_FROM_RAW
	TransferData::TransferData(
		std::vector< std::byte >&& source,
		const std::shared_ptr< BufferSuballocationHandle >& target,
		const vk::DeviceSize target_offset,
		const vk::DeviceSize source_offset ) :
	  m_type( TransferType::SUBALLOCATION_FROM_RAW ),
	  m_source( std::forward< std::vector< std::byte > >( source ) ),
	  m_target( target ),
	  m_target_offset( target_offset ),
	  m_source_offset( source_offset )
	{
		markBad();
	}

	//! IMAGE_FROM_BUFFER
	TransferData::TransferData(
		const std::shared_ptr< BufferSuballocationHandle >& source, const std::shared_ptr< ImageHandle >& target ) :
	  m_type( TransferType::IMAGE_FROM_SUBALLOCATION ),
	  m_source( source ),
	  m_target( target ),
	  m_target_offset( 0 ),
	  m_source_offset( 0 )
	{
		markBad();
	}

	//! IMAGE_FROM_RAW
	TransferData::TransferData( std::vector< std::byte >&& source, const std::shared_ptr< ImageHandle >& target ) :
	  m_type( TransferType::IMAGE_FROM_RAW ),
	  m_source( std::forward< std::vector< std::byte > >( source ) ),
	  m_target( target ),
	  m_target_offset( 0 ),
	  m_source_offset( 0 )
	{
		assert( !std::get< RawData >( m_source ).empty() );
		markBad();
	}

} // namespace fgl::engine::memory
