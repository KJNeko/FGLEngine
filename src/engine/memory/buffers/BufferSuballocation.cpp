//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocation.hpp"

#include "BufferSuballocationHandle.hpp"
#include "SuballocationView.hpp"
#include "align.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::memory
{

	BufferSuballocation::BufferSuballocation( std::shared_ptr< BufferSuballocationHandle > handle ) :
	  m_handle( std::move( handle ) ),
	  m_offset( m_handle->m_offset ),
	  m_byte_size( m_handle->m_size )
	{
		if ( handle.use_count() > 30 ) throw std::runtime_error( "AAAAAAAAA" );
	}

	BufferSuballocation& BufferSuballocation::operator=( BufferSuballocation&& other ) noexcept
	{
		m_handle = std::move( other.m_handle );

		m_offset = m_handle->m_offset;
		m_byte_size = m_handle->m_size;

		other.m_offset = 0;
		other.m_byte_size = 0;

		other.m_handle = nullptr;

		return *this;
	}

	BufferSuballocation::BufferSuballocation( BufferSuballocation&& other ) noexcept :
	  m_handle( std::move( other.m_handle ) ),
	  m_offset( m_handle->m_offset ),
	  m_byte_size( m_handle->m_size )
	{
		other.m_offset = 0;
		other.m_byte_size = 0;

		other.m_handle = nullptr;
	}

	void* BufferSuballocation::ptr() const
	{
		assert( m_handle != nullptr );
		FGL_ASSERT( m_handle->m_ptr, "Buffer must be mappable to use `ptr()`" );
		return m_handle->m_ptr;
	}

	void BufferSuballocation::flush( const vk::DeviceSize beg, const vk::DeviceSize end ) const
	{
		assert( beg < end );
		assert( m_handle != nullptr );
		assert( m_handle->m_ptr != nullptr && "BufferSuballocationT::flush() called before map()" );
		assert( end <= this->m_byte_size );

		vk::MappedMemoryRange range {};
		range.memory = m_handle->m_parent_buffer->getMemory();
		range.offset = m_offset + beg;

		const vk::DeviceSize min_atom_size { Device::getInstance().m_properties.limits.nonCoherentAtomSize };
		const vk::DeviceSize size { end - beg };

		assert( size > 0 );

		range.size = align( size, min_atom_size );

		assert( range.size > 0 );

		if ( range.size > m_byte_size ) range.size = VK_WHOLE_SIZE;

		const std::vector< vk::MappedMemoryRange > ranges { range };

		Device::getInstance()->flushMappedMemoryRanges( ranges );
	}

	Buffer& BufferSuballocation::getBuffer() const
	{
		assert( m_handle != nullptr );

		return m_handle->m_parent_buffer;
	}

	vk::Buffer BufferSuballocation::getVkBuffer() const
	{
		assert( m_handle != nullptr );

		return m_handle->m_parent_buffer->getVkBuffer();
	}

	vk::DescriptorBufferInfo BufferSuballocation::descriptorInfo( const std::size_t byte_offset ) const
	{
		assert( !std::isnan( m_offset ) );
		assert( !std::isnan( m_byte_size ) );

		FGL_ASSERT( byte_offset < m_byte_size, "Byte offset was greater then byte size!" );
		FGL_ASSERT(
			m_offset + byte_offset < this->getBuffer()->size(),
			"Byte offset + buffer offset was greater then parent buffer size" );

		return { getVkBuffer(), m_offset + byte_offset, m_byte_size };
	}

	BufferSuballocation::~BufferSuballocation() = default;

	SuballocationView BufferSuballocation::view( const vk::DeviceSize offset, const vk::DeviceSize size ) const
	{
		assert( m_handle != nullptr );
		assert( offset + size <= m_byte_size && "BufferSuballocation::view() called with offset + size > m_size" );

		return { m_handle, offset, size };
	}

	BufferSuballocation::BufferSuballocation( const Buffer& buffer, const vk::DeviceSize size ) :
	  BufferSuballocation( buffer->allocate( size, 1 ) )
	{}

	BufferSuballocation::
		BufferSuballocation( const Buffer& buffer, const std::size_t t_size, const std::uint32_t t_align ) :
	  BufferSuballocation( buffer->allocate( t_size, t_align ) )
	{}

} // namespace fgl::engine::memory
