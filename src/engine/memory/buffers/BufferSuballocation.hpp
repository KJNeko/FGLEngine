//
// Created by kj16609 on 12/23/23.
//

#pragma once

#include "BufferSuballocationHandle.hpp"
#include "debug/Track.hpp"
#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine::memory
{
	class BufferHandle;
	class SuballocationView;

	struct BufferSuballocationHandle;

	class BufferSuballocation
	{
		std::shared_ptr< BufferSuballocationHandle > m_handle;

		debug::Track< "GPU", "BufferSuballocation" > m_track {};

		friend class TransferManager;

	  protected:

		void flush() const;

		explicit BufferSuballocation( const Buffer& buffer, std::size_t t_size, std::uint32_t t_align );

	  public:

		using value_type = void;

		BufferSuballocation() = delete;

		BufferSuballocation( std::shared_ptr< BufferSuballocationHandle > handle );
		BufferSuballocation( const Buffer& buffer, vk::DeviceSize size );

		FGL_DELETE_COPY( BufferSuballocation );

		BufferSuballocation( BufferSuballocation&& other ) noexcept;
		BufferSuballocation& operator=( BufferSuballocation&& other ) noexcept;

		SuballocationView view( vk::DeviceSize offset, vk::DeviceSize size ) const;

		//! Returns true when the buffer has been staged by the StagingManager
		bool ready() const { return m_handle->ready(); }

		//! Returns a mapped pointer.
		/**
		 * @note If the buffer this suballocation is from is unable to be mapped then this function will return nullptr
		 */
		void* ptr() const;

		//! Returns the total byte size of the allocation
		vk::DeviceSize bytesize() const noexcept { return m_handle->m_size; }

		Buffer& getBuffer() const;

		vk::Buffer getVkBuffer() const;

		vk::DeviceSize getOffset() const noexcept { return m_handle->m_offset; }

		vk::DescriptorBufferInfo descriptorInfo( std::size_t byte_offset = 0 ) const;

		const std::shared_ptr< BufferSuballocationHandle >& getHandle() { return m_handle; }

		void setRebindInfoStorage(
			const std::shared_ptr< descriptors::DescriptorSet >& descriptor, std::uint32_t binding_idx ) const;

		void setRebindInfoUniform(
			const std::shared_ptr< descriptors::DescriptorSet >& descriptor, std::uint32_t binding_idx ) const;

		void setRebindInfoArray(
			std::uint32_t binding_idx,
			const std::shared_ptr< descriptors::DescriptorSet >& descriptor,
			std::size_t array_idx,
			std::size_t item_size ) const;

		~BufferSuballocation();
	};

} // namespace fgl::engine::memory