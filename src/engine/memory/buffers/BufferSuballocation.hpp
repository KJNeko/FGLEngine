//
// Created by kj16609 on 12/23/23.
//

#pragma once

#include "BufferSuballocationHandle.hpp"
#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine::memory
{
	class Buffer;
	class SuballocationView;

	struct BufferSuballocationHandle;

	class BufferSuballocation
	{
		std::shared_ptr< BufferSuballocationHandle > m_handle;

		friend class TransferManager;

	  protected:

		vk::DeviceSize m_offset;
		vk::DeviceSize m_byte_size;

		void flush( vk::DeviceSize beg, vk::DeviceSize end ) const;

		explicit BufferSuballocation( Buffer& buffer, std::size_t t_size, std::uint32_t t_align );

	  public:

		using value_type = void;

		BufferSuballocation() = delete;

		BufferSuballocation( std::shared_ptr< BufferSuballocationHandle > handle );
		BufferSuballocation( Buffer& buffer, const vk::DeviceSize size );

		BufferSuballocation( const BufferSuballocation& ) = delete;
		BufferSuballocation& operator=( const BufferSuballocation& ) = delete;

		BufferSuballocation( BufferSuballocation&& other ) noexcept;
		BufferSuballocation& operator=( BufferSuballocation&& other ) noexcept;

		SuballocationView view( const vk::DeviceSize offset, const vk::DeviceSize size ) const;

		//! Returns true when the buffer has been staged by the StagingManager
		bool ready() const { return m_handle->ready(); }

		//! Returns a mapped pointer.
		/**
		 * @note If the buffer this suballocation is from is unable to be mapped then this function will return nullptr
		 */
		void* ptr() const;

		//! Returns the total byte size of the allocation
		vk::DeviceSize bytesize() const noexcept { return m_byte_size; }

		Buffer& getBuffer() const;

		vk::Buffer getVkBuffer() const;

		vk::DeviceSize getOffset() const noexcept { return m_offset; }

		vk::DescriptorBufferInfo descriptorInfo( std::size_t byte_offset = 0 ) const;

		const std::shared_ptr< BufferSuballocationHandle >& getHandle() { return m_handle; }

		~BufferSuballocation() = default;
	};

} // namespace fgl::engine::memory