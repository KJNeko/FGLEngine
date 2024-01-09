//
// Created by kj16609 on 12/23/23.
//

#pragma once

#include "Buffer.hpp"
#include "engine/Device.hpp"
#include "engine/concepts/is_suballocation.hpp"

namespace fgl::engine
{

	class BufferSuballocation
	{
	  protected:

		Buffer& m_buffer;
		BufferSuballocationInfo m_info {};

		void* m_mapped { nullptr };

		void flush( vk::DeviceSize beg, vk::DeviceSize end )
		{
			assert( m_mapped != nullptr && "BufferSuballocationT::flush() called before map()" );
			vk::MappedMemoryRange range {};
			range.memory = m_buffer.getMemory();
			range.offset = m_info.offset + beg;

			const vk::DeviceSize min_atom_size { Device::getInstance().m_properties.limits.nonCoherentAtomSize };
			const auto size { end - beg };

			range.size = align( size, min_atom_size );

			if ( range.size > m_info.size ) range.size = VK_WHOLE_SIZE;

			if ( Device::getInstance().device().flushMappedMemoryRanges( 1, &range ) != vk::Result::eSuccess )
				throw std::runtime_error( "Failed to flush memory" );
		}

		BufferSuballocation& operator=( BufferSuballocation&& other )
		{
			//Free ourselves if we are valid
			if ( this->m_info.offset != std::numeric_limits< decltype( m_info.offset ) >::max() )
				m_buffer.free( m_info );

			//Take their info
			m_info = other.m_info;

			//Set other to be invalid
			other.m_info.offset = std::numeric_limits< decltype( m_info.offset ) >::max();
			other.m_info.size = 0;

			return *this;
		}

	  public:

		using value_type = void;

		BufferSuballocation() = delete;
		BufferSuballocation( const BufferSuballocation& ) = delete;
		BufferSuballocation& operator=( const BufferSuballocation& ) = delete;

		BufferSuballocation( BufferSuballocation&& other ) : m_buffer( other.m_buffer )
		{
			if ( this->m_info.offset != std::numeric_limits< decltype( m_info.offset ) >::max() )
				m_buffer.free( m_info );

			m_info = other.m_info;
			m_mapped = other.m_mapped;

			other.m_info.offset = std::numeric_limits< decltype( m_info.offset ) >::max();
			other.m_info.size = 0;
			other.m_mapped = nullptr;
		}

		BufferSuballocation( Buffer& buffer, const std::size_t memory_size, const std::uint32_t alignment = 1 ) :
		  m_buffer( buffer ),
		  m_info( buffer.suballocate( memory_size, alignment ) ),
		  m_mapped( m_buffer.isMappable() ? buffer.map( m_info ) : nullptr )
		{
			assert( memory_size != 0 && "BufferSuballocation::BufferSuballocation() called with memory_size == 0" );
		}

		BufferSuballocation(
			std::unique_ptr< Buffer >& buffer_ptr, const std::size_t memory_size, const std::uint32_t alignment = 1 ) :
		  BufferSuballocation( *buffer_ptr.get(), memory_size, alignment )
		{
			assert( buffer_ptr != nullptr && "BufferSuballocation::BufferSuballocation() called with nullptr" );
		}

		vk::DeviceSize size() const { return m_info.size; }

		Buffer& getBuffer() const { return m_buffer; }

		vk::Buffer getVkBuffer() const { return m_buffer.getBuffer(); }

		vk::DeviceSize getOffset() const { return m_info.offset; }

		vk::DeviceSize offset() const { return m_info.offset; }

		vk::DescriptorBufferInfo descriptorInfo() const
		{
			vk::DescriptorBufferInfo info {};
			info.buffer = m_buffer.getBuffer();
			info.offset = m_info.offset;
			info.range = m_info.size;
			return info;
		}

		~BufferSuballocation()
		{
			if ( m_info.offset != std::numeric_limits< decltype( m_info.offset ) >::max() ) m_buffer.free( m_info );
		}
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

		HostSingleT( Buffer& buffer ) : BufferSuballocation( buffer, sizeof( T ), alignof( T ) ) {}

		HostSingleT& operator=( T& t )
		{
			ZoneScoped;

			*static_cast< T* >( this->m_mapped ) = t;

			flush();

			return *this;
		}

		void flush() { BufferSuballocation::flush( 0, this->m_info.size ); }
	};

	template < typename T >
	concept is_typed_suballocation = requires( T t ) {
		{
			t.operator->()
		} -> std::same_as< typename T::value_type* >;
		requires is_buffer< typename T::BufferT >;
	};

} // namespace fgl::engine