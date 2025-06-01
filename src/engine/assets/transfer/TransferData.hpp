//
// Created by kj16609 on 7/2/24.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "engine/memory/buffers/BufferHandle.hpp"
#include "rendering/CommandBufferPool.hpp"

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

	namespace memory
	{
		struct BufferSuballocationHandle;
		class BufferHandle;
	} // namespace memory

	namespace gui
	{
		void drawTransferManager();
	}

} // namespace fgl::engine

namespace fgl::engine::memory
{
	//! <Source, Target>
	using CopyRegionKey = std::pair< std::shared_ptr< BufferHandle >, std::shared_ptr< BufferHandle > >;

	struct BufferHasher
	{
		std::size_t operator()( const std::shared_ptr< BufferHandle >& buffer ) const;
	};

	struct CopyRegionKeyHasher
	{
		std::size_t operator()( const std::pair< std::shared_ptr< BufferHandle >, std::shared_ptr< BufferHandle > >&
		                            pair ) const;
	};

	using CopyRegionMap = std::unordered_map< CopyRegionKey, std::vector< vk::BufferCopy >, CopyRegionKeyHasher >;

	//! Data store for staging operations
	class TransferData
	{
		//! Type of transfer this data represents
		enum class TransferType
		{
			IMAGE_FROM_RAW,
			IMAGE_FROM_SUBALLOCATION,
			SUBALLOCATION_FROM_SUBALLOCATION,
			SUBALLOCATION_FROM_RAW,
		} m_type;

		using RawData = std::vector< std::byte >;
		using TransferSuballocationHandle = std::shared_ptr< BufferSuballocationHandle >;
		using TransferImageHandle = std::shared_ptr< ImageHandle >;

		using SourceData = std::variant< RawData, TransferImageHandle, TransferSuballocationHandle >;
		using TargetData = std::variant< TransferImageHandle, TransferSuballocationHandle >;

		//! Source data. Data type depends on m_type
		SourceData m_source;

		//! Target data. Data type depends on m_type
		TargetData m_target;

		vk::DeviceSize m_target_offset;
		vk::DeviceSize m_source_offset;

		//! Performs copy of raw data to the staging buffer
		bool convertRawToBuffer( Buffer& staging_buffer );

		bool performImageStage( CommandBuffer& cmd_buffer, std::uint32_t transfer_idx, std::uint32_t graphics_idx );

		//! Same as @ref performImageStage Performs extra step of copying data to a staging buffer
		/** @note After calling this function m_type will be `IMAGE_FROM_BUFFER`
		 */
		bool performRawImageStage(
			CommandBuffer& buffer, Buffer& staging_buffer, std::uint32_t transfer_idx, std::uint32_t graphics_idx );

		bool performSuballocationStage( CopyRegionMap& copy_regions );

		//! Barrier that ensures that the source is properly written to before it's read to the target
		vk::BufferMemoryBarrier readSourceBarrier();

		//! Barrier that ensures the target is written to before allowing reads
		vk::BufferMemoryBarrier readTargetBarrier();

		[[nodiscard]] bool targetIsHostVisible() const;
		[[nodiscard]] bool sourceIsHostVisible() const;
		[[nodiscard]] bool targetNeedsFlush() const;

		//! Same as @ref performBufferStage Performs extra step of copying data to a staging buffer. If the target is host visible, Then this function will do nothing and return true.
		/** @note After calling this function m_type will be `BUFFER_FROM_BUFFER`
		 */
		bool performRawSuballocationStage(
			CommandBuffer& cmd_buffer, Buffer& staging_buffer, CopyRegionMap& copy_regions );

		friend class TransferManager;

	  public:

		TransferData() = delete;

		TransferData( const TransferData& ) = default;
		TransferData& operator=( const TransferData& ) = default;

		TransferData( TransferData&& other ) = default;
		TransferData& operator=( TransferData&& ) = default;

		bool stage(
			CommandBuffer& buffer,
			Buffer& staging_buffer,
			CopyRegionMap& copy_regions,
			std::uint32_t transfer_idx,
			std::uint32_t graphics_idx );

		bool isReady() const;

		//! Marks the target as not staged/not ready
		void markBad();

		//! Marks the target as staged/ready
		void markGood();

		void cleanupSource();

		TransferType type() { return m_type; };

		SourceData& source() { return m_source; }

		TargetData& target() { return m_target; }

		vk::DeviceSize targetOffset() const { return m_target_offset; }

		vk::DeviceSize sourceOffset() const { return m_source_offset; }

		//SUBALLOCATION_FROM_SUBALLOCATION
		TransferData(
			const std::shared_ptr< BufferSuballocationHandle >& source,
			const std::shared_ptr< BufferSuballocationHandle >& target,
			vk::DeviceSize target_offset = 0,
			vk::DeviceSize source_offset = 0 );

		// SUBALLOCATION_FROM_RAW
		TransferData(
			std::vector< std::byte >&& source,
			const std::shared_ptr< BufferSuballocationHandle >& target,
			vk::DeviceSize target_offset = 0,
			vk::DeviceSize source_offset = 0 );

		//IMAGE_FROM_X
		TransferData(
			const std::shared_ptr< BufferSuballocationHandle >& source, const std::shared_ptr< ImageHandle >& target );

		TransferData( std::vector< std::byte >&& source, const std::shared_ptr< ImageHandle >& target );
	};

} // namespace fgl::engine::memory