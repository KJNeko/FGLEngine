//
// Created by kj16609 on 12/23/23.
//

#pragma once

#include "engine/concepts/is_suballocation.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{
	class Buffer;
	class BufferHandle;
	class SuballocationView;

	struct BufferSuballocationHandle;

	class BufferSuballocation
	{
		std::shared_ptr< BufferSuballocationHandle > m_handle;

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

		void* ptr() const;

		vk::DeviceSize bytesize() const noexcept { return m_byte_size; }

		Buffer& getBuffer() const;

		vk::Buffer getVkBuffer() const;

		vk::DeviceSize getOffset() const noexcept { return m_offset; }

		vk::DescriptorBufferInfo descriptorInfo() const;

		~BufferSuballocation() = default;
	};

	//! Single element allocation of T
	template < typename T >
	struct HostSingleT final : public BufferSuballocation
	{
		using value_type = T;

		HostSingleT() = delete;
		HostSingleT( const HostSingleT& ) = delete;
		HostSingleT( HostSingleT&& ) = delete;
		HostSingleT& operator=( const HostSingleT& ) = delete;

		HostSingleT( Buffer& buffer ) : BufferSuballocation( buffer.suballocate( sizeof( T ), alignof( T ) ) ) {}

		HostSingleT& operator=( T& t )
		{
			*static_cast< T* >( this->ptr() ) = t;

			flush();

			return *this;
		}

		void flush() { BufferSuballocation::flush( 0, this->m_byte_size ); }
	};

	template < typename T >
	concept is_typed_suballocation = requires( T t ) {
		{
			t.operator->()
		} -> std::same_as< typename T::value_type* >;
		requires is_buffer< typename T::BufferT >;
	};

} // namespace fgl::engine