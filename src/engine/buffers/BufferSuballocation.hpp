//
// Created by kj16609 on 12/23/23.
//

#pragma once

#include "BufferSuballocationHandle.hpp"
#include "engine/concepts/is_suballocation.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine::memory
{
	class Buffer;
	class BufferHandle;
	class SuballocationView;

	struct BufferSuballocationHandle;

	class BufferSuballocation
	{
		std::shared_ptr< BufferSuballocationHandle > m_handle;

		friend class TransferManager;

	  protected:

		vk::DeviceSize m_offset;
		vk::DeviceSize m_byte_size;

		void flush( vk::DeviceSize beg, vk::DeviceSize end );

	  public:

		using value_type = void;

		BufferSuballocation() = delete;

		BufferSuballocation( std::shared_ptr< BufferSuballocationHandle > handle );
		BufferSuballocation( Buffer& buffer, const vk::DeviceSize size );

		BufferSuballocation( const BufferSuballocation& ) = delete;
		BufferSuballocation& operator=( const BufferSuballocation& ) = delete;

		BufferSuballocation( BufferSuballocation&& other );
		BufferSuballocation& operator=( BufferSuballocation&& other );

		SuballocationView view( const vk::DeviceSize offset, const vk::DeviceSize size ) const;

		bool ready() const { return m_handle->ready(); }

		void* ptr() const;

		vk::DeviceSize bytesize() const noexcept { return m_byte_size; }

		Buffer& getBuffer() const;

		vk::Buffer getVkBuffer() const;

		vk::DeviceSize getOffset() const noexcept { return m_offset; }

		vk::DescriptorBufferInfo descriptorInfo() const;

		const std::shared_ptr< BufferSuballocationHandle >& getHandle() { return m_handle; }

		~BufferSuballocation() = default;
	};



	template < typename T >
	concept is_typed_suballocation = requires( T t ) {
		{
			t.operator->()
		} -> std::same_as< typename T::value_type* >;
		requires is_buffer< typename T::BufferT >;
	};

} // namespace fgl::engine