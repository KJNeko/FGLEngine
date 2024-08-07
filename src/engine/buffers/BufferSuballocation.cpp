//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocation.hpp"

#include "Buffer.hpp"
#include "BufferSuballocationHandle.hpp"
#include "SuballocationView.hpp"
#include "align.hpp"
#include "engine/logging/logging.hpp"

namespace fgl::engine::memory
{

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

	BufferSuballocation::BufferSuballocation( std::shared_ptr< BufferSuballocationHandle > handle ) :
	  m_handle( std::move( handle ) ),
	  m_offset( m_handle->m_offset ),
	  m_byte_size( m_handle->m_size )
	{}

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
		return m_handle->mapped;
	}

	void BufferSuballocation::flush( vk::DeviceSize beg, vk::DeviceSize end )
	{
		assert( beg < end );
		assert( m_handle != nullptr );
		assert( m_handle->mapped != nullptr && "BufferSuballocationT::flush() called before map()" );
		assert( end <= this->m_byte_size );

		vk::MappedMemoryRange range {};
		range.memory = m_handle->buffer.getMemory();
		range.offset = m_offset + beg;

		const vk::DeviceSize min_atom_size { Device::getInstance().m_properties.limits.nonCoherentAtomSize };
		const vk::DeviceSize size { end - beg };

		assert( size > 0 );

		range.size = align( size, min_atom_size );

		assert( range.size > 0 );

		if ( range.size > m_byte_size ) range.size = VK_WHOLE_SIZE;

		std::vector< vk::MappedMemoryRange > ranges { range };

		Device::getInstance()->flushMappedMemoryRanges( ranges );
	}

	Buffer& BufferSuballocation::getBuffer() const
	{
		assert( m_handle != nullptr );

		return m_handle->buffer;
	}

	vk::Buffer BufferSuballocation::getVkBuffer() const
	{
		assert( m_handle != nullptr );

		return m_handle->buffer.getVkBuffer();
	}

	vk::DescriptorBufferInfo BufferSuballocation::descriptorInfo() const
	{
		assert( !std::isnan( m_offset ) );
		assert( !std::isnan( m_byte_size ) );

		return vk::DescriptorBufferInfo( getVkBuffer(), m_offset, m_byte_size );
	}

	SuballocationView BufferSuballocation::view( const vk::DeviceSize offset, const vk::DeviceSize size ) const
	{
		assert( m_handle != nullptr );
		assert( offset + size <= m_byte_size && "BufferSuballocation::view() called with offset + size > m_size" );

		return { m_handle, offset, size };
	}

	BufferSuballocation::BufferSuballocation( Buffer& buffer, const vk::DeviceSize size ) :
	  BufferSuballocation( buffer.allocate( size ) )
	{}

} // namespace fgl::engine
