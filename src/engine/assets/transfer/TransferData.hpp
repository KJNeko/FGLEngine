//
// Created by kj16609 on 7/2/24.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "engine/memory/buffers/BufferHandle.hpp"

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

	namespace memory
	{
		struct BufferSuballocationHandle;
		class BufferHandle;
	} // namespace memory
} // namespace fgl::engine

namespace fgl::engine::memory
{
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
			eImageFromRaw,
			eImageFromBuffer,
			eBufferFromBuffer,
			eBufferFromRaw
		} m_type;

		using RawData = std::vector< std::byte >;
		using TransferBufferHandle = std::shared_ptr< BufferSuballocationHandle >;
		using TransferImageHandle = std::shared_ptr< ImageHandle >;

		using SourceData = std::variant< RawData, TransferBufferHandle, TransferImageHandle >;
		using TargetData = std::variant< TransferBufferHandle, TransferImageHandle >;

		//! Source data. Data type depends on m_type
		SourceData m_source;
		vk::DeviceSize m_source_offset;

		//! Target data. Data type depends on m_type
		TargetData m_target;
		vk::DeviceSize m_target_offset;

		vk::DeviceSize m_size;

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
			std::uint32_t transfer_idx,
			std::uint32_t graphics_idx );

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

		bool stage(
			vk::raii::CommandBuffer& buffer,
			Buffer& staging_buffer,
			CopyRegionMap& copy_regions,
			std::uint32_t transfer_idx,
			std::uint32_t graphics_idx );

		//! Marks the target as not staged/not ready
		void markBad() const;

		//! Marks the target as staged/ready
		void markGood() const;

		//BUFFER_FROM_X
		TransferData(
			const std::shared_ptr< BufferSuballocationHandle >& source,
			const std::shared_ptr< BufferSuballocationHandle >& target,
			vk::DeviceSize size = 0,
			vk::DeviceSize dst_offset = 0,
			vk::DeviceSize src_offset = 0 );

		TransferData(
			std::vector< std::byte >&& source,
			const std::shared_ptr< BufferSuballocationHandle >& target,
			vk::DeviceSize size = 0,
			vk::DeviceSize dst_offset = 0,
			vk::DeviceSize src_offset = 0 );

		//IMAGE_FROM_X
		TransferData(
			const std::shared_ptr< BufferSuballocationHandle >& source,
			const std::shared_ptr< ImageHandle >& target,
			vk::DeviceSize size = 0,
			vk::DeviceSize src_offset = 0 );

		TransferData(
			std::vector< std::byte >&& source,
			const std::shared_ptr< ImageHandle >& target,
			vk::DeviceSize size = 0,
			vk::DeviceSize src_offset = 0 );
	};

} // namespace fgl::engine::memory