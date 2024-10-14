//
// Created by kj16609 on 6/26/24.
//

#include "TransferManager.hpp"

#include "engine/assets/image/Image.hpp"
#include "engine/assets/image/ImageHandle.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/memory/buffers/Buffer.hpp"
#include "engine/memory/buffers/BufferSuballocation.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"

namespace fgl::engine::memory
{
	void TransferManager::recordCommands( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;
		//Keep inserting new commands until we fill up the staging buffer

		if ( queue.size() > 0 ) log::info( "[TransferManager]: Queue size: {}", queue.size() );

		std::size_t counter { 0 };
		constexpr std::size_t counter_max { 256 };

		while ( queue.size() > 0 )
		{
			++counter;
			if ( counter > counter_max ) break;

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

		if ( counter > 0 ) log::debug( "Queued {} objects for transfer", counter );

		const std::vector< vk::BufferMemoryBarrier > from_memory_barriers { createFromGraphicsBarriers() };

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

		const std::vector< vk::BufferMemoryBarrier > to_buffer_memory_barriers { createFromTransferBarriers() };

		// Release the buffer regions back to the graphics queue
		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader,
			vk::DependencyFlags(),
			{},
			to_buffer_memory_barriers,
			{} );
	}

	void TransferManager::resizeBuffer( const std::uint64_t size )
	{
		staging_buffer = std::make_unique< Buffer >(
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
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

	void TransferManager::copyToVector( BufferVector& source, BufferVector& target, const std::size_t target_offset )
	{
		TransferData transfer_data { source.getHandle(), target.getHandle(), target_offset };

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
	  cmd_buffer_allocinfo( Device::getInstance().getCommandPool(), vk::CommandBufferLevel::ePrimary, 1 ),
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

} // namespace fgl::engine::memory
