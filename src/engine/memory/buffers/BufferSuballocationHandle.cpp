//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocationHandle.hpp"

#include "Buffer.hpp"
#include "BufferSuballocation.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::memory
{
	vk::Buffer BufferSuballocationHandle::getBuffer() const
	{
		return buffer.getVkBuffer();
	}

	BufferSuballocationHandle::
		BufferSuballocationHandle( Buffer& p_buffer, const vk::DeviceSize offset, const vk::DeviceSize memory_size ) :
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

	vk::BufferCopy BufferSuballocationHandle::
		copyRegion( const BufferSuballocationHandle& target, const std::size_t offset ) const
	{
		vk::BufferCopy copy {};
		copy.size = std::min( this->m_size, target.m_size );
		copy.srcOffset = this->getOffset();
		copy.dstOffset = target.getOffset() + offset;
		return copy;
	}

	void BufferSuballocationHandle::copyTo(
		const vk::raii::CommandBuffer& cmd_buffer,
		const BufferSuballocationHandle& other,
		const std::size_t offset ) const
	{
		const vk::BufferCopy copy_region { copyRegion( other, offset ) };

		const std::vector< vk::BufferCopy > copy_regions { copy_region };

		cmd_buffer.copyBuffer( this->getVkBuffer(), other.getVkBuffer(), copy_regions );
	}

} // namespace fgl::engine::memory