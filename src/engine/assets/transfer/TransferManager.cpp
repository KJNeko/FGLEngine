//
// Created by kj16609 on 6/26/24.
//

#include "TransferManager.hpp"

#include "engine/assets/image/Image.hpp"
#include "engine/assets/image/ImageHandle.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/memory/buffers/BufferHandle.hpp"
#include "engine/memory/buffers/BufferSuballocation.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"

#ifdef ENABLE_IMGUI
#include "imgui.h"
#endif

namespace fgl::engine::memory
{
	void TransferManager::recordCommands( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;
		//Keep inserting new commands until we fill up the staging buffer

		if ( !m_queue.empty() ) log::info( "[TransferManager]: Queue size: {}", m_queue.size() );

		std::size_t counter { 0 };
		constexpr std::size_t counter_max { 256 };

		while ( !m_queue.empty() )
		{
			++counter;
			if ( counter > counter_max ) break;

			TransferData data { std::move( m_queue.front() ) };
			m_queue.pop();

			if ( data.stage(
					 command_buffer,
					 m_staging_buffer,
					 m_copy_regions,
					 m_transfer_queue_index,
					 m_graphics_queue_index ) )
			{
				m_processing.emplace_back( std::move( data ) );
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

		vk::DebugUtilsLabelEXT debug_label {};
		debug_label.pLabelName = "Transfer";

		command_buffer.beginDebugUtilsLabelEXT( debug_label );

		// Acquire the buffer from the queue family
		command_buffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			from_memory_barriers,
			{} );

		//Record all the buffer copies
		for ( auto& [ key, regions ] : m_copy_regions )
		{
			const auto& [ source, target ] = key;

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

		command_buffer.endDebugUtilsLabelEXT();
	}

	void TransferManager::resizeBuffer( const std::uint64_t size )
	{
		m_staging_buffer.resize( size );
	}

	void TransferManager::copySuballocationRegion(
		const std::shared_ptr< BufferSuballocationHandle >& src,
		const std::shared_ptr< BufferSuballocationHandle >& dst,
		const vk::DeviceSize size,
		const vk::DeviceSize dst_offset,
		const std::size_t src_offset )
	{
		FGL_ASSERT( src->size() == dst->size(), "Source and destination suballocations must be the same size" );

		//! If the buffer has not been staged, Then there is nothing to copy in the first place.
		//! If the source is not stable, Then it might be staged in the future.
		if ( !src->ready() ) return;

		TransferData transfer_data { src, dst, size, dst_offset, src_offset };

		m_queue.emplace( std::move( transfer_data ) );
	}

	void TransferManager::submitBuffer( const vk::raii::CommandBuffer& command_buffer ) const
	{
		ZoneScoped;

		std::vector< vk::Fence > fences { m_completion_fence };

		Device::getInstance()->resetFences( fences );

		command_buffer.end();

		vk::SubmitInfo info {};

		std::vector< vk::CommandBuffer > buffers { *command_buffer };

		std::vector< vk::Semaphore > sems { m_transfer_semaphore };
		info.setSignalSemaphores( sems );
		info.setCommandBuffers( buffers );

		m_transfer_queue.submit( info, m_completion_fence );
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createFromGraphicsBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : m_copy_regions )
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
				barrier.srcQueueFamilyIndex = m_graphics_queue_index;
				barrier.dstQueueFamilyIndex = m_transfer_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createToTransferBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : m_copy_regions )
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
				barrier.srcQueueFamilyIndex = m_graphics_queue_index;
				barrier.dstQueueFamilyIndex = m_transfer_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createFromTransferBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( auto& [ key, regions ] : m_copy_regions )
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
				barrier.srcQueueFamilyIndex = m_transfer_queue_index;
				barrier.dstQueueFamilyIndex = m_graphics_queue_index;

				barriers.emplace_back( barrier );
			}
		}

		return barriers;
	}

	std::vector< vk::BufferMemoryBarrier > TransferManager::createToGraphicsBarriers()
	{
		std::vector< vk::BufferMemoryBarrier > barriers {};

		for ( const auto& [ key, regions ] : m_copy_regions )
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
			vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader,
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
		assert( data.size() > 0 );
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
	  m_cmd_buffer_allocinfo( Device::getInstance().getCommandPool(), vk::CommandBufferLevel::ePrimary, 1 ),
	  m_transfer_buffers( Device::getInstance().device().allocateCommandBuffers( m_cmd_buffer_allocinfo ) ),
	  m_completion_fence( device->createFence( {} ) )
	{
		log::info( "Transfer manager created with size {}", literals::size_literals::toString( buffer_size ) );

		GLOBAL_TRANSFER_MANAGER = this;
		m_staging_buffer->setDebugName( "Staging buffer" );
	}

	void TransferManager::submitNow()
	{
		ZoneScoped;

		m_allow_transfers = false;

		auto& transfer_buffer { m_transfer_buffers[ 0 ] };
		transfer_buffer.reset();

		vk::CommandBufferBeginInfo info {};

		transfer_buffer.begin( info );

		recordCommands( transfer_buffer );

		submitBuffer( transfer_buffer );

		if ( m_processing.size() > 0 )
		{
			log::debug(
				"Submitted {} objects to be transfered, Transfer buffer usage: {}",
				m_processing.size(),
				literals::size_literals::toString( m_staging_buffer->used() ) );
		}

		for ( auto& processed : m_processing )
		{
			processed.markGood();
		}

		//Drop the data
		m_processing.clear();
	}

	void TransferManager::drawImGui() const
	{
#ifdef ENABLE_IMGUI
		ImGui::Text( "|- %s Allocated", literals::size_literals::toString( m_staging_buffer->size() ).c_str() );
		ImGui::Text( "|- %s Used ", literals::size_literals::toString( m_staging_buffer->used() ).c_str() );
		ImGui::Text(
			"|- %s Unused",
			literals::size_literals::toString( m_staging_buffer->size() - m_staging_buffer->used() ).c_str() );

		ImGui::Text( "|- %i transfer remaining", m_queue.size() );
		ImGui::Text( "|- %zu objects being processed", m_processing.size() );
		ImGui::Text( "|- %zu total copy regions", m_copy_regions.size() );
#endif
	}

} // namespace fgl::engine::memory
