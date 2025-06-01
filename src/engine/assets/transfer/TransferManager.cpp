//
// Created by kj16609 on 6/26/24.
//

#include "TransferManager.hpp"

#include <list>

#include "engine/assets/image/Image.hpp"
#include "engine/assets/image/ImageHandle.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/memory/buffers/BufferHandle.hpp"
#include "engine/memory/buffers/BufferSuballocation.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"

namespace fgl::engine::memory
{
	void TransferManager::recordCommands( CommandBuffer& command_buffer )
	{
		ZoneScoped;
		//Keep inserting new commands until we fill up the staging buffer

		if ( !m_queue.empty() ) log::info( "[TransferManager]: Queue size: {}", m_queue.size() );

		std::size_t counter { 0 };

		while ( !m_queue.empty() && counter < constants::TRANSFER_LIMIT )
		{
			TransferData data { std::move( m_queue.front() ) };
			m_queue.pop();

			if ( !data.isReady() )
			{
				continue;
				m_queue.push( std::move( data ) );
			}

			if ( data.stage(
					 command_buffer,
					 m_staging_buffer,
					 m_copy_regions,
					 m_transfer_queue_index,
					 m_graphics_queue_index ) )
			{
				m_processing.emplace_back( std::move( data ) );
				++counter;
			}
			else
			{
				// We were unable to stage for a reason
				log::debug( "Unable to stage object. Breaking out of loop, Objects will be staged next pass" );
				m_queue.push( data );
				break;
			}
		}

		if ( counter > 0 ) log::debug( "Queued {} objects for transfer", counter );

		const std::vector< vk::BufferMemoryBarrier > from_memory_barriers { createFromGraphicsBarriers() };

		{
			vk::DebugUtilsLabelEXT debug_label {};
			debug_label.pLabelName = "Transfer";

			command_buffer->beginDebugUtilsLabelEXT( debug_label );
		}

		// Acquire the buffer from the queue family
		command_buffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			from_memory_barriers,
			{} );

		std::list< std::shared_ptr< BufferHandle > > copy_order {};
		using Key = decltype( m_copy_regions )::key_type;
		using Regions = decltype( m_copy_regions )::mapped_type;

		std::vector< std::pair< Key, Regions > > sorted_regions {};

		for ( const auto& [ key, regions ] : m_copy_regions )
		{
			const auto& [ source, target ] = key;

			// The copy order depends on the source. In this case the source m_old_buffer should be targeted first if it has any copies associated with it
			auto old_weak { source->m_old_handle };

			while ( !old_weak.expired() )
			{
				copy_order.insert( copy_order.end(), old_weak.lock() );
			}

			copy_order.insert( copy_order.end(), source );
			sorted_regions.emplace_back( key, regions );
		}

		std::ranges::sort(
			sorted_regions,
			[ &copy_order ]( const auto& left, const auto& right ) -> bool
			{
				const auto left_itter { std::ranges::find( copy_order, left.first.first ) };
				const auto right_itter { std::ranges::find( copy_order, right.first.first ) };

				return std::distance( left_itter, copy_order.begin() )
			         < std::distance( right_itter, copy_order.begin() );
			} );

		//Record all the buffer copies
		for ( auto& [ key, regions ] : sorted_regions )
		{
			const auto& [ source, target ] = key;

			vk::DebugUtilsLabelEXT debug_label {};
			const std::string str { std::format( "Copy: {} -> {}", source->m_debug_name, target->m_debug_name ) };
			debug_label.pLabelName = str.c_str();
			command_buffer->beginDebugUtilsLabelEXT( debug_label );

			std::vector< vk::BufferMemoryBarrier > barriers {};

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = source->getVkBuffer();
				barrier.offset = region.srcOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
				barriers.emplace_back( barrier );
			}

			command_buffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eTransfer,
				vk::DependencyFlags(),
				{},
				barriers,
				{} );

			command_buffer->copyBuffer( source->getVkBuffer(), target->getVkBuffer(), regions );
			command_buffer->endDebugUtilsLabelEXT();
		}

		const std::vector< vk::BufferMemoryBarrier > to_buffer_memory_barriers { createFromTransferBarriers() };

		// Release the buffer regions back to the graphics queue
		command_buffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader
				| vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			{},
			to_buffer_memory_barriers,
			{} );

		command_buffer->endDebugUtilsLabelEXT();
	}

	void TransferManager::resizeBuffer( const std::uint64_t size )
	{
		m_staging_buffer->resize( size );
	}

	void TransferManager::copySuballocationRegion(
		const std::shared_ptr< BufferSuballocationHandle >& src,
		const std::shared_ptr< BufferSuballocationHandle >& dst )
	{
		FGL_ASSERT( src->m_size == dst->m_size, "Source and destination suballocations must be the same size" );
		FGL_ASSERT( src->m_parent_buffer->m_debug_name != "Debug name", "Buffers should likely be properly named!" );
		FGL_ASSERT( dst->m_parent_buffer->m_debug_name != "Debug name", "Buffers should likely be properly named!" );

		TransferData transfer_data { src, dst };

		m_queue.emplace( std::move( transfer_data ) );
	}

	void TransferManager::submitBuffer( CommandBuffer& command_buffer )
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { m_completion_fence };

		Device::getInstance()->resetFences( fences );

		command_buffer->end();

		vk::SubmitInfo info {};

		std::vector< vk::CommandBuffer > buffers { *command_buffer };

		std::vector< vk::Semaphore > sems { m_transfer_semaphore };
		info.setSignalSemaphores( sems );
		info.setCommandBuffers( buffers );

		m_transfer_queue.submit( info, m_completion_fence );
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createFromGraphicsBarriers()
	{
		ZoneScoped;
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : m_copy_regions )
		{
			const auto& [ source, target ] = key;

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = target->getVkBuffer();
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.srcQueueFamilyIndex = m_graphics_queue_index;
				barrier.dstQueueFamilyIndex = m_transfer_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createToTransferBarriers()
	{
		ZoneScoped;
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : m_copy_regions )
		{
			auto& [ source, target ] = key;

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = target->getVkBuffer();
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eNone;
				barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.srcQueueFamilyIndex = m_graphics_queue_index;
				barrier.dstQueueFamilyIndex = m_transfer_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createFromTransferBarriers()
	{
		ZoneScoped;
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : m_copy_regions )
		{
			auto& [ source, target ] = key;

			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};
				barrier.buffer = target->getVkBuffer();
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				barrier.dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eIndexRead
				                      | vk::AccessFlagBits::eShaderRead;
				barrier.srcQueueFamilyIndex = m_transfer_queue_index;
				barrier.dstQueueFamilyIndex = m_graphics_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createToGraphicsBarriers()
	{
		ZoneScoped;
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( const auto& [ key, regions ] : m_copy_regions )
		{
			const auto& [ src, dst ] = key;
			for ( const auto& region : regions )
			{
				vk::BufferMemoryBarrier barrier {};

				barrier.buffer = dst->getVkBuffer();
				barrier.offset = region.dstOffset;
				barrier.size = region.size;
				barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
				barrier.dstAccessMask = vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead;
				barrier.srcQueueFamilyIndex = m_transfer_queue_index;
				barrier.dstQueueFamilyIndex = m_graphics_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	inline static TransferManager* GLOBAL_TRANSFER_MANAGER {};

	void TransferManager::takeOwnership( CommandBuffer& command_buffer )
	{
		std::vector< vk::BufferMemoryBarrier > barriers { createToTransferBarriers() };

		command_buffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eTransfer, {}, {}, barriers, {} );
	}

	void TransferManager::recordOwnershipTransferDst( CommandBuffer& command_buffer )
	{
		ZoneScoped;

		std::vector< vk::BufferMemoryBarrier > barriers { createToGraphicsBarriers() };

		command_buffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader
				| vk::PipelineStageFlagBits::eComputeShader,
			{},
			{},
			barriers,
			{} );
	}

	void TransferManager::dump()
	{
		//Block on fence

		std::vector< vk::Fence > fences { m_completion_fence };

		(void)Device::getInstance()->waitForFences( fences, VK_TRUE, std::numeric_limits< std::size_t >::max() );

		m_processing.clear();
		m_copy_regions.clear();
		m_allow_transfers = true;
	}

	TransferManager& TransferManager::getInstance()
	{
		assert( GLOBAL_TRANSFER_MANAGER );
		return *GLOBAL_TRANSFER_MANAGER;
	}

	void TransferManager::copyToVector( BufferVector& source, BufferVector& target, const std::size_t target_offset )
	{
		TransferData transfer_data { source.getHandle(), target.getHandle(), target_offset };

		m_queue.emplace( std::move( transfer_data ) );
	}

	void TransferManager::copyToImage( std::vector< std::byte >&& data, const Image& image )
	{
		assert( !data.empty() );
		TransferData transfer_data { std::forward< std::vector< std::byte > >( data ), image.m_handle };

		assert( std::get< TransferData::RawData >( transfer_data.m_source ).size() > 0 );

		m_queue.emplace( std::move( transfer_data ) );
	}

	TransferManager::TransferManager( Device& device, const vk::DeviceSize buffer_size ) :
	  m_staging_buffer(
		  buffer_size,
		  vk::BufferUsageFlagBits::eTransferSrc,
		  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent ),
	  m_transfer_queue_index( device.phyDevice()
	                              .queueInfo()
	                              .getIndex( vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics ) ),
	  m_graphics_queue_index( device.phyDevice().queueInfo().getIndex( vk::QueueFlagBits::eGraphics ) ),
	  m_transfer_queue( device->getQueue( m_transfer_queue_index, 0 ) ),
	  m_transfer_semaphore( device->createSemaphore( {} ) ),
	  m_cmd_buffer_allocinfo(),
	  m_transfer_buffers(
		  Device::getInstance().getCmdBufferPool().getCommandBuffers( 1, CommandBufferHandle::Primary ) ),
	  m_completion_fence( device->createFence( {} ) )
	{
		log::info( "Transfer manager created with size {}", literals::size_literals::toString( buffer_size ) );

		m_staging_buffer->setDebugName( "Transfer buffer" );

		GLOBAL_TRANSFER_MANAGER = this;
	}

	void TransferManager::submitNow()
	{
		ZoneScoped;

		m_allow_transfers = false;

		auto& transfer_buffer { m_transfer_buffers[ 0 ] };
		transfer_buffer->reset();

		constexpr vk::CommandBufferBeginInfo info {};

		transfer_buffer->begin( info );

		recordCommands( transfer_buffer );

		submitBuffer( transfer_buffer );

		if ( !m_processing.empty() ) log::debug( "Submitted {} objects to be transfered", m_processing.size() );

		for ( auto& processed : m_processing )
		{
			processed.markGood();
			processed.cleanupSource();
		}

		//Drop the data
		m_processing.clear();
	}

} // namespace fgl::engine::memory
