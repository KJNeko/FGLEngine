//
// Created by kj16609 on 7/2/24.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "engine/buffers/BufferSuballocationHandle.hpp"
#include "engine/literals/size.hpp"
#include "engine/logging/logging.hpp"
#include "engine/utils.hpp"

namespace vk
{
	namespace raii
	{
		class CommandBuffer;
	}
	struct BufferCopy;
	class Buffer;
} // namespace vk

namespace fgl::engine
{
	class Texture;
	class ImageHandle;
	class Image;
} // namespace fgl::engine

namespace fgl::engine::memory
{
	class Buffer;

	//! <Source, Target>
	using CopyRegionKey = std::pair< vk::Buffer, vk::Buffer >;

	struct BufferHasher
	{
		std::size_t operator()( const vk::Buffer& buffer ) const;
	};

	struct CopyRegionKeyHasher
	{
		std::size_t operator()( const std::pair< vk::Buffer, vk::Buffer >& pair ) const;
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

} // namespace fgl::engine::memory