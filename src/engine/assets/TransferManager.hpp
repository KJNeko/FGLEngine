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

	// <Source,Target>
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

	class TransferData
	{
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

		SourceData m_source;
		TargetData m_target;

		bool performImageStage(
			vk::raii::CommandBuffer& cmd_buffer, std::uint32_t transfer_idx, std::uint32_t graphics_idx );

		bool performRawImageStage(
			vk::raii::CommandBuffer& buffer,
			Buffer& staging_buffer,
			std::uint32_t graphics_idx,
			std::uint32_t transfer_idx );

		bool performBufferStage( CopyRegionMap& copy_regions );

		bool performRawBufferStage( Buffer& staging_buffer, CopyRegionMap& copy_regions );

		bool convertRawToBuffer( Buffer& );

		friend class TransferManager;

	  public:

		TransferData() = delete;

		TransferData( const TransferData& ) = default;
		TransferData& operator=( const TransferData& ) = default;

		TransferData( TransferData&& other ) = default;
		TransferData& operator=( TransferData&& ) = default;

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

	class TransferManager
	{
		//TODO: Ring Buffer
		std::queue< TransferData > queue {};

		std::vector< TransferData > processing {};

		// std::thread transfer_thread;

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

		void takeOwnership( vk::raii::CommandBuffer& buffer );
		//! Records the barriers required for transfering queue ownership
		void recordOwnershipTransferDst( vk::raii::CommandBuffer& command_buffer );

		//! Drops the processed items
		void dump();

		//! Prepares the staging buffer. Filling it as much as possible
		void prepareStaging();

		static void createInstance( Device& device, std::uint64_t buffer_size );
		static TransferManager& getInstance();

		TransferManager( Device& device, std::uint64_t buffer_size );

		FGL_DELETE_ALL_Ro5( TransferManager );

		void resizeBuffer( const std::uint64_t size )
		{
			staging_buffer = std::make_unique< Buffer >(
				size,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		}

		//! Queues an buffer to be transfered
		template < typename BufferT >
			requires is_device_vector< BufferT >
		void copyToBuffer( std::vector< std::byte >&& data, BufferT& buffer )
		{
			assert( data.size() > 0 );
			TransferData transfer_data { std::forward< std::vector< std::byte > >( data ), buffer.m_handle };

			queue.emplace( std::move( transfer_data ) );
		}

		template < typename T, typename BufferT >
			requires is_device_vector< BufferT >
		void copyToBuffer( const std::vector< T >& data, BufferT& buffer )
		{
			assert( data.size() > 0 );
			std::vector< std::byte > punned_data {};
			punned_data.resize( sizeof( T ) * data.size() );

			std::memcpy( punned_data.data(), data.data(), sizeof( T ) * data.size() );

			copyToBuffer( std::move( punned_data ), buffer );
		}

		void copyToBuffer( BufferVector& source, BufferVector& target );

		void copyToImage( std::vector< std::byte >&& data, Image& image );

		//! Forces the queue to be submitted now before the buffer is filled.
		void submitNow();
	};

} // namespace fgl::engine
