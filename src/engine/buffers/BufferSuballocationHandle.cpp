//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocationHandle.hpp"

#include "Buffer.hpp"

namespace fgl::engine
{
	BufferSuballocationHandle::
		BufferSuballocationHandle( Buffer& p_buffer, vk::DeviceSize offset, vk::DeviceSize memory_size ) :
	  buffer( p_buffer ),
	  m_size( memory_size ),
	  m_offset( offset ),
	  mapped( buffer.map( *this ) )
	{
		assert( memory_size != 0 && "BufferSuballocation::BufferSuballocation() called with memory_size == 0" );
	}

	vk::Buffer BufferSuballocationHandle::getVkBuffer() const
	{
		return buffer.getVkBuffer();
	}

	BufferSuballocationHandle::~BufferSuballocationHandle()
	{
		buffer.free( *this );
	}

} // namespace fgl::engine