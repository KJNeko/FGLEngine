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
	class Image;
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

		//! Buffer used for any raw -> buffer transfers
		Buffer m_staging_buffer;

		//! Queue of data needing to be transfered and submitted.
		std::queue< TransferData > m_queue {};

		//! Data actively in flight (Submitted to the DEVICE transfer queue)
		std::vector< TransferData > m_processing {};

		//! Map to store copy regions for processing vectors
		CopyRegionMap m_copy_regions {};

		std::uint32_t m_transfer_queue_index;
		std::uint32_t m_graphics_queue_index;
		vk::raii::Queue m_transfer_queue;

		//! Signaled once a transfer completes
		vk::raii::Semaphore m_transfer_semaphore;

		vk::CommandBufferAllocateInfo m_cmd_buffer_allocinfo;

		std::vector< vk::raii::CommandBuffer > m_transfer_buffers;

		vk::raii::Fence m_completion_fence;

		//! True if transfers would be performed before the start of the next frame
		bool m_allow_transfers { true };

		void recordCommands( vk::raii::CommandBuffer& command_buffer );

		void submitBuffer( const vk::raii::CommandBuffer& command_buffer ) const;

		//! Creates barriers that releases ownership from the graphics family to the transfer queue.
		std::vector< vk::BufferMemoryBarrier > createFromGraphicsBarriers();

		//! Returns barriers that acquires ownership from the graphics family to the transfer queue
		std::vector< vk::BufferMemoryBarrier > createToTransferBarriers();

		//! Returns barriers that releases ownership from the transfer family to the graphics family
		std::vector< vk::BufferMemoryBarrier > createFromTransferBarriers();

		//! Creates barriers that acquires ownership from the transfer family to the graphics family
		std::vector< vk::BufferMemoryBarrier > createToGraphicsBarriers();

	  public:

		TransferManager( Device& device, vk::DeviceSize buffer_size );

		FGL_DELETE_ALL_RO5( TransferManager );

		vk::raii::Semaphore& getFinishedSem() { return m_transfer_semaphore; }

		//! Takes ownership of memory regions from the graphics queue via memory barriers.
		void takeOwnership( CommandBuffer& command_buffer );

		//! Records the barriers required for transfering queue ownership
		void recordOwnershipTransferDst( CommandBuffer& command_buffer );

		//! Drops the processed items
		void dump();

		static TransferManager& getInstance();

		//! Resizes the staging buffer.
		void resizeBuffer( std::uint64_t size );

		void copySuballocationRegion(
			const std::shared_ptr< BufferSuballocationHandle >& src,
			const std::shared_ptr< BufferSuballocationHandle >& dst,
			vk::DeviceSize size = 0,
			vk::DeviceSize dst_offset = 0,
			std::size_t src_offset = 0 );

		//! Queues a buffer to be transfered
		template < typename DeviceVectorT >
			requires is_device_vector< DeviceVectorT >
		void copyToVector(
			std::vector< std::byte >&& data,
			DeviceVectorT& device_vector,
			const vk::DeviceSize size = 0,
			const vk::DeviceSize dst_offset = 0,
			const vk::DeviceSize src_offset = 0 )
		{
			assert( !data.empty() );
			TransferData transfer_data {
				std::forward< std::vector< std::byte > >( data ), device_vector.m_handle, size, dst_offset, src_offset
			};

			m_queue.emplace( std::move( transfer_data ) );
		}

		template < typename T, typename DeviceVectorT >
			requires is_device_vector< DeviceVectorT > && std::same_as< T, typename DeviceVectorT::Type >
		void copyToVector( const T& t, const std::size_t idx, DeviceVectorT& device_vector )
		{
			std::vector< std::byte > data {};
			data.resize( sizeof( T ) );

			assert( idx < device_vector.bytesize() / sizeof( T ) );

			std::memcpy( data.data(), &t, sizeof( T ) );

			const auto size { sizeof( T ) };
			const auto dst_offset { idx * size };

			copyToVector( std::move( data ), device_vector, size, dst_offset );
		}

		//! Queues a data copy from a STL vector to a device vector
		template < typename T, typename DeviceVectorT >
			requires is_device_vector< DeviceVectorT > && std::same_as< T, typename DeviceVectorT::Type >
		void copyToVector( const std::vector< T >& data, DeviceVectorT& device_vector )
		{
			assert( data.size() > 0 );
			std::vector< std::byte > punned_data {};
			punned_data.resize( sizeof( T ) * data.size() );

			std::memcpy( punned_data.data(), data.data(), sizeof( T ) * data.size() );

			copyToVector( std::move( punned_data ), device_vector, punned_data.size() );
		}

		void copyToVector( BufferVector& source, BufferVector& target, std::size_t target_offset );

		void copyToImage( std::vector< std::byte >&& data, const Image& image );

		//! Forces the queue to be submitted now before the buffer is filled.
		void submitNow();

		void drawImGui() const;
	};

} // namespace fgl::engine::memory
