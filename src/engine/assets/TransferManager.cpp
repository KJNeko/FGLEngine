//
// Created by kj16609 on 6/26/24.
//

#include "TransferManager.hpp"

#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/buffers/exceptions.hpp"
#include "engine/buffers/vector/HostVector.hpp"
#include "engine/image/Image.hpp"
#include "engine/image/ImageHandle.hpp"
#include "engine/literals/size.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine::memory
{
	void TransferManager::recordCommands( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;
		//Keep inserting new commands until we fill up the staging buffer

		if ( queue.size() > 0 ) log::info( "[TransferManager]: Queue size: {}", queue.size() );

		while ( queue.size() > 0 )
		{
			TransferData data { std::move( queue.front() ) };
			queue.pop();

			if ( data.stage(
					 command_buffer, *staging_buffer, copy_regions, transfer_queue_index, graphics_queue_index ) )
			{
				processing.emplace_back( std::move( data ) );
			}
			else
			{
				// We were unable to stage for a reason
				log::info( "Unable to stage object. Breaking out of loop" );
				queue.push( data );
				break;
			}
		}

		std::vector< vk::BufferMemoryBarrier > from_memory_barriers { createFromGraphicsBarriers() };

		// Acquire the buffer from the queue family
		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			from_memory_barriers,
			{} );

		//Record all the buffer copies
		for ( auto& [ key, regions ] : copy_regions )
		{
			auto& [ source, target ] = key;

			command_buffer.copyBuffer( source, target, regions );
		}

		std::vector< vk::BufferMemoryBarrier > to_buffer_memory_barriers { createFromTransferBarriers() };

		// Release the buffer regions back to the graphics queue
		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader,
			vk::DependencyFlags(),
			{},
			to_buffer_memory_barriers,
			{} );
	}

	void TransferManager::submitBuffer( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { completion_fence };

		Device::getInstance()->resetFences( fences );

		command_buffer.end();

		vk::SubmitInfo info {};

		std::vector< vk::CommandBuffer > buffers { *command_buffer };

		std::vector< vk::Semaphore > sems { transfer_semaphore };
		info.setSignalSemaphores( sems );
		info.setCommandBuffers( buffers );

		transfer_queue.submit( info, completion_fence );
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createFromGraphicsBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : copy_regions )
		{
			auto& [ source, target ] = key;

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = target;
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.srcQueueFamilyIndex = graphics_queue_index;
				barrier.dstQueueFamilyIndex = transfer_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createToTransferBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : copy_regions )
		{
			auto& [ source, target ] = key;

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = target;
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.srcQueueFamilyIndex = graphics_queue_index;
				barrier.dstQueueFamilyIndex = transfer_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createFromTransferBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : copy_regions )
		{
			auto& [ source, target ] = key;

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = target;
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eIndexRead;
				barrier.srcQueueFamilyIndex = transfer_queue_index;
				barrier.dstQueueFamilyIndex = graphics_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createToGraphicsBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( const auto& [ key, regions ] : copy_regions )
		{
			const auto& [ src, dst ] = key;
			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};

				barrier.buffer = dst;
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
				barrier.dstAccessMask = vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead;
				barrier.srcQueueFamilyIndex = transfer_queue_index;
				barrier.dstQueueFamilyIndex = graphics_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	inline static std::unique_ptr< TransferManager > global_transfer_manager {};

	void TransferManager::takeOwnership( vk::raii::CommandBuffer& command_buffer )
	{
		std::vector< vk::BufferMemoryBarrier > barriers { createToTransferBarriers() };

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eTransfer, {}, {}, barriers, {} );
	}

	void TransferManager::recordOwnershipTransferDst( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;

		std::vector< vk::BufferMemoryBarrier > barriers { createToGraphicsBarriers() };

		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader,
			{},
			{},
			barriers,
			{} );
	}

	void TransferManager::dump()
	{
		//Block on fence

		std::vector< vk::Fence > fences { completion_fence };

		(void)Device::getInstance()->waitForFences( fences, VK_TRUE, std::numeric_limits< std::size_t >::max() );

		processing.clear();
		copy_regions.clear();
	}

	void TransferManager::createInstance( Device& device, std::uint64_t buffer_size )
	{
		log::info(
			"Transfer manager created with a buffer size of {}",
			fgl::literals::size_literals::to_string( buffer_size ) );
		global_transfer_manager = std::make_unique< TransferManager >( device, buffer_size );
	}

	TransferManager& TransferManager::getInstance()
	{
		assert( global_transfer_manager );
		return *global_transfer_manager;
	}

	void TransferManager::copyToVector( BufferVector& source, BufferVector& target )
	{
		TransferData transfer_data { source.getHandle(), target.getHandle() };

		queue.emplace( std::move( transfer_data ) );
	}

	void TransferManager::copyToImage( std::vector< std::byte >&& data, Image& image )
	{
		assert( data.size() > 0 );
		TransferData transfer_data { std::forward< std::vector< std::byte > >( data ), image.m_handle };

		assert( std::get< TransferData::RawData >( transfer_data.m_source ).size() > 0 );

		queue.emplace( std::move( transfer_data ) );

		log::debug( "[TransferManager]: Queue size now {}", queue.size() );
	}

	TransferManager::TransferManager( Device& device, std::uint64_t buffer_size ) :
	  transfer_queue_index( device.phyDevice()
	                            .queueInfo()
	                            .getIndex( vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics ) ),
	  graphics_queue_index( device.phyDevice().queueInfo().getIndex( vk::QueueFlagBits::eGraphics ) ),
	  transfer_queue( device->getQueue( transfer_queue_index, 0 ) ),
	  transfer_semaphore( device->createSemaphore( {} ) ),
	  transfer_buffers( Device::getInstance().device().allocateCommandBuffers( cmd_buffer_allocinfo ) ),
	  completion_fence( device->createFence( {} ) )
	{
		resizeBuffer( buffer_size );
	}

	void TransferManager::submitNow()
	{
		ZoneScoped;

		auto& transfer_buffer { transfer_buffers[ 0 ] };
		transfer_buffer.reset();

		vk::CommandBufferBeginInfo info {};

		transfer_buffer.begin( info );

		recordCommands( transfer_buffer );

		submitBuffer( transfer_buffer );

		if ( processing.size() > 0 ) log::debug( "Submitted {} objects to be transfered", processing.size() );

		for ( auto& processed : processing )
		{
			processed.markGood();
		}

		//Drop the data
		processing.clear();
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
		std::uint32_t transfer_idx,
		std::uint32_t graphics_idx )
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

		const auto copy_info { source->copyRegion( *target ) };

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
		log::debug( "Raw buffer -> Buffer staging" );
		if ( !convertRawToBuffer( staging_buffer ) ) return false;
		return performBufferStage( copy_regions );
	}

	bool TransferData::convertRawToBuffer( Buffer& staging_buffer )
	{
		// Prepare the staging buffer first.
		assert( std::holds_alternative< RawData >( m_source ) );
		assert( std::get< RawData >( m_source ).size() > 0 );

		try
		{
			HostVector< std::byte > vector { staging_buffer, std::get< RawData >( m_source ) };

			m_source = vector.getHandle();

			return true;
		}
		catch ( BufferOOM )
		{
			log::warn( "Staging buffer full. Aborting stage" );
			return false;
		}

		std::unreachable();
	}

	bool TransferData::stage(
		vk::raii::CommandBuffer& buffer,
		Buffer& staging_buffer,
		CopyRegionMap& copy_regions,
		std::uint32_t transfer_idx,
		std::uint32_t graphics_idx )
	{
		ZoneScoped;
		switch ( m_type )
		{
			default:
				throw std::runtime_error( "Invalid transfer type" );
			case IMAGE_FROM_RAW:
				return performRawImageStage( buffer, staging_buffer, transfer_idx, graphics_idx );
			case IMAGE_FROM_BUFFER:
				return performImageStage( buffer, transfer_idx, graphics_idx );
			case BUFFER_FROM_RAW:
				return performRawBufferStage( staging_buffer, copy_regions );
			case BUFFER_FROM_BUFFER:
				return performBufferStage( copy_regions );
		}

		std::unreachable();
	}

	void TransferData::markBad()
	{
		switch ( m_type )
		{
			case BUFFER_FROM_RAW:
				[[fallthrough]];
			case BUFFER_FROM_BUFFER:
				std::get< TransferBufferHandle >( m_target )->setReady( false );
				break;
			case IMAGE_FROM_RAW:
				[[fallthrough]];
			case IMAGE_FROM_BUFFER:
				std::get< TransferImageHandle >( m_target )->setReady( false );
		}
	}

	void TransferData::markGood()
	{
		switch ( m_type )
		{
			case BUFFER_FROM_RAW:
				[[fallthrough]];
			case BUFFER_FROM_BUFFER:
				std::get< TransferBufferHandle >( m_target )->setReady( true );
				break;
			case IMAGE_FROM_RAW:
				[[fallthrough]];
			case IMAGE_FROM_BUFFER:
				std::get< TransferImageHandle >( m_target )->setReady( true );
		}
	}

} // namespace fgl::engine
