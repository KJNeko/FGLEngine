//
// Created by kj16609 on 6/26/24.
//

#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <functional>
#include <queue>
#include <thread>

#include "engine/FGL_DEFINES.hpp"
#include "engine/buffers/Buffer.hpp"
#include "engine/buffers/BufferSuballocationHandle.hpp"
#include "engine/buffers/vector/concepts.hpp"
#include "engine/image/ImageHandle.hpp"
#include "engine/literals/size.hpp"
#include "engine/utils.hpp"

namespace fgl::engine
{
	class BufferVector;
	class Texture;
	class ImageHandle;
	struct BufferSuballocationHandle;

	class Image;
	class BufferSuballocation;

	//! <Source, Target>
	using CopyRegionKey = std::pair< vk::Buffer, vk::Buffer >;

	struct BufferHasher
	{
		std::size_t operator()( const vk::Buffer& buffer ) const
		{
			return reinterpret_cast< std::size_t >( static_cast< VkBuffer >( buffer ) );
		}
	};

	struct CopyRegionKeyHasher
	{
		std::size_t operator()( const std::pair< vk::Buffer, vk::Buffer >& pair ) const
		{
			const std::size_t hash_a { BufferHasher {}( std::get< 0 >( pair ) ) };
			const std::size_t hash_b { BufferHasher {}( std::get< 1 >( pair ) ) };

			std::size_t seed { 0 };
			fgl::engine::hashCombine( seed, hash_a, hash_b );
			return seed;
		}
	};

	using CopyRegionMap = std::unordered_map< CopyRegionKey, std::vector< vk::BufferCopy >, CopyRegionKeyHasher >;

	//! Data store for staging operations
	class TransferData
	{
		//! Type of transfer this data represents
		enum TransferType
		{
			IMAGE_FROM_RAW,
			IMAGE_FROM_BUFFER,
			BUFFER_FROM_BUFFER,
			BUFFER_FROM_RAW
		} m_type;

		using RawData = std::vector< std::byte >;
		using TransferBufferHandle = std::shared_ptr< BufferSuballocationHandle >;
		using TransferImageHandle = std::shared_ptr< ImageHandle >;

		using SourceData = std::variant< RawData, TransferBufferHandle, TransferImageHandle >;
		using TargetData = std::variant< TransferBufferHandle, TransferImageHandle >;

		//! Source data. Data type depends on m_type
		SourceData m_source;

		//! Target data. Data type depends on m_type
		TargetData m_target;

		//! Performs copy of raw data to the staging buffer
		bool convertRawToBuffer( Buffer& staging_buffer );

		bool performImageStage(
			vk::raii::CommandBuffer& cmd_buffer, std::uint32_t transfer_idx, std::uint32_t graphics_idx );

		//! Same as @ref performImageStage Performs extra step of copying data to a staging buffer
		/** @note After calling this function m_type will be `IMAGE_FROM_BUFFER`
		 */
		bool performRawImageStage(
			vk::raii::CommandBuffer& buffer,
			Buffer& staging_buffer,
			std::uint32_t graphics_idx,
			std::uint32_t transfer_idx );

		bool performBufferStage( CopyRegionMap& copy_regions );

		//! Same as @ref performBufferStage Performs extra step of copying data to a staging buffer
		/** @note After calling this function m_type will be `BUFFER_FROM_BUFFER`
		 */
		bool performRawBufferStage( Buffer& staging_buffer, CopyRegionMap& copy_regions );

		friend class TransferManager;

	  public:

		TransferData() = delete;

		TransferData( const TransferData& ) = default;
		TransferData& operator=( const TransferData& ) = default;

		TransferData( TransferData&& other ) = default;
		TransferData& operator=( TransferData&& ) = default;

		//! BUFFER_FROM_BUFFER
		TransferData(
			const std::shared_ptr< BufferSuballocationHandle >& source,
			const std::shared_ptr< BufferSuballocationHandle >& target ) :
		  m_type( BUFFER_FROM_BUFFER ),
		  m_source( source ),
		  m_target( target )
		{
			log::debug(
				"[TransferManager]: Queued buffer -> buffer transfer: {}",
				fgl::literals::size_literals::to_string( source->m_size ) );
			markBad();
		}

		//! BUFFER_FROM_RAW
		TransferData( std::vector< std::byte >&& source, const std::shared_ptr< BufferSuballocationHandle >& target ) :
		  m_type( BUFFER_FROM_RAW ),
		  m_source( std::forward< std::vector< std::byte > >( source ) ),
		  m_target( target )
		{
			log::debug(
				"[TransferManager]: Queued raw -> buffer transfer: {}",
				literals::size_literals::to_string( std::get< RawData >( m_source ).size() ) );
			assert( std::get< RawData >( m_source ).size() > 0 );
			markBad();
		}

		//! IMAGE_FROM_BUFFER
		TransferData(
			const std::shared_ptr< BufferSuballocationHandle >& source, const std::shared_ptr< ImageHandle >& target ) :
		  m_type( IMAGE_FROM_BUFFER ),
		  m_source( source ),
		  m_target( target )
		{
			log::debug(
				"[TransferManager]: Queued image -> image transfer: {}",
				fgl::literals::size_literals::to_string( source->m_size ) );
			markBad();
		}

		//! IMAGE_FROM_RAW
		TransferData( std::vector< std::byte >&& source, const std::shared_ptr< ImageHandle >& target ) :
		  m_type( IMAGE_FROM_RAW ),
		  m_source( std::forward< std::vector< std::byte > >( source ) ),
		  m_target( target )
		{
			log::debug(
				"[TransferManager]: Queued raw -> image transfer: {}",
				literals::size_literals::to_string( std::get< RawData >( m_source ).size() ) );
			assert( std::get< RawData >( m_source ).size() > 0 );
			markBad();
		}

		bool stage(
			vk::raii::CommandBuffer& buffer,
			Buffer& staging_buffer,
			CopyRegionMap& copy_regions,
			std::uint32_t transfer_idx,
			std::uint32_t graphics_idx );

		//! Marks the target as not staged/not ready
		void markBad();

		//! Marks the target as staged/ready
		void markGood();
	};

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

		vk::CommandBufferAllocateInfo cmd_buffer_allocinfo { Device::getInstance().getCommandPool(),
			                                                 vk::CommandBufferLevel::ePrimary,
			                                                 1 };

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

		vk::raii::Semaphore& getFinishedSem() { return transfer_semaphore; }

		//! Takes ownership of memory regions from the graphics queue via memory barriers.
		void takeOwnership( vk::raii::CommandBuffer& buffer );

		//! Records the barriers required for transfering queue ownership
		void recordOwnershipTransferDst( vk::raii::CommandBuffer& command_buffer );

		//! Drops the processed items
		void dump();

		static void createInstance( Device& device, std::uint64_t buffer_size );
		static TransferManager& getInstance();

		TransferManager( Device& device, std::uint64_t buffer_size );

		FGL_DELETE_ALL_Ro5( TransferManager );

		//! Resizes the staging buffer.
		void resizeBuffer( const std::uint64_t size )
		{
			staging_buffer = std::make_unique< Buffer >(
				size,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		}

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

} // namespace fgl::engine
