//
// Created by kj16609 on 6/26/24.
//

#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <functional>
#include <queue>

#include "TransferData.hpp"
#include "engine/FGL_DEFINES.hpp"
#include "engine/memory/buffers/vector/concepts.hpp"

namespace fgl::engine
{
	class Device;

	namespace memory
	{
		class BufferVector;

		struct BufferSuballocationHandle;

		class BufferSuballocation;
	} // namespace memory
} // namespace fgl::engine

namespace fgl::engine::memory
{

	//! Manages transfers from HOST (CPU) to DEVICE (GPU)
	class TransferManager
	{
		//TODO: Ring Buffer
		//! Queue of data needing to be transfered and submitted.
		std::queue< TransferData > queue {};

		//! Data actively in flight (Submitted to the DEVICE transfer queue)
		std::vector< TransferData > processing {};

		//! Buffer used for any raw -> buffer transfers
		std::unique_ptr< Buffer > staging_buffer {};

	  private:

		//! Map to store copy regions for processing vectors
		CopyRegionMap copy_regions {};

		std::uint32_t transfer_queue_index;
		std::uint32_t graphics_queue_index;
		vk::raii::Queue transfer_queue;

		//! Signaled once a transfer completes
		vk::raii::Semaphore transfer_semaphore;

		vk::CommandBufferAllocateInfo cmd_buffer_allocinfo;

		std::vector< vk::raii::CommandBuffer > transfer_buffers;

		vk::raii::Fence completion_fence;

		void recordCommands( vk::raii::CommandBuffer& command_buffer );

		void submitBuffer( vk::raii::CommandBuffer& command_buffer );

		//! Creates barriers that releases ownership from the graphics family to the transfer queue.
		std::vector< vk::BufferMemoryBarrier > createFromGraphicsBarriers();

		//! Returns barriers that acquires ownership from the graphics family to the transfer queue
		std::vector< vk::BufferMemoryBarrier > createToTransferBarriers();

		//! Returns barriers that releases ownership from the transfer family to the graphics family
		std::vector< vk::BufferMemoryBarrier > createFromTransferBarriers();

		//! Creates barriers that acquires ownership from the transfer family to the graphics family
		std::vector< vk::BufferMemoryBarrier > createToGraphicsBarriers();

	  public:

		TransferManager( Device& device, std::uint64_t buffer_size );

		FGL_DELETE_ALL_Ro5( TransferManager );

		vk::raii::Semaphore& getFinishedSem() { return transfer_semaphore; }

		//! Takes ownership of memory regions from the graphics queue via memory barriers.
		void takeOwnership( vk::raii::CommandBuffer& buffer );

		//! Records the barriers required for transfering queue ownership
		void recordOwnershipTransferDst( vk::raii::CommandBuffer& command_buffer );

		//! Drops the processed items
		void dump();

		static void createInstance( Device& device, std::uint64_t buffer_size );
		static TransferManager& getInstance();

		//! Resizes the staging buffer.
		void resizeBuffer( std::uint64_t size );

		//! Queues a buffer to be transfered
		template < typename DeviceVectorT >
			requires is_device_vector< DeviceVectorT >
		void copyToVector( std::vector< std::byte >&& data, DeviceVectorT& device_vector )
		{
			assert( data.size() > 0 );
			TransferData transfer_data { std::forward< std::vector< std::byte > >( data ), device_vector.m_handle };

			queue.emplace( std::move( transfer_data ) );
		}

		//! Queues a data copy from a STL vector to a device vector
		template < typename T, typename DeviceVectorT >
			requires is_device_vector< DeviceVectorT >
		void copyToVector( const std::vector< T >& data, DeviceVectorT& device_vector )
		{
			assert( data.size() > 0 );
			std::vector< std::byte > punned_data {};
			punned_data.resize( sizeof( T ) * data.size() );

			std::memcpy( punned_data.data(), data.data(), sizeof( T ) * data.size() );

			copyToVector( std::move( punned_data ), device_vector );
		}

		void copyToVector( BufferVector& source, BufferVector& target );

		void copyToImage( std::vector< std::byte >&& data, Image& image );

		//! Forces the queue to be submitted now before the buffer is filled.
		void submitNow();
	};

} // namespace fgl::engine::memory
