//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocationHandle.hpp"

#include "Buffer.hpp"
#include "BufferSuballocation.hpp"
#include "engine/logging/logging.hpp"

namespace fgl::engine::memory
{
	vk::Buffer BufferSuballocationHandle::getBuffer()
	{
		return buffer.getBuffer();
	}

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

	vk::BufferCopy BufferSuballocationHandle::copyRegion( BufferSuballocationHandle& target )
	{
		vk::BufferCopy copy {};
		copy.size = std::min( this->m_size, target.m_size );
		copy.srcOffset = this->getOffset();
		copy.dstOffset = target.getOffset();

		log::debug(
			"Created buffer copy of size {} from offset [{:X}]:{} to [{:X}]:{}",
			copy.size,
			reinterpret_cast< std::size_t >( static_cast< VkBuffer >( this->getVkBuffer() ) ),
			copy.srcOffset,
			reinterpret_cast< std::size_t >( static_cast< VkBuffer >( target.getVkBuffer() ) ),
			copy.dstOffset );

		return copy;
	}

	void BufferSuballocationHandle::copyTo( vk::raii::CommandBuffer& cmd_buffer, BufferSuballocationHandle& other )
	{
		vk::BufferCopy copy_region { copyRegion( other ) };

		std::vector< vk::BufferCopy > copy_regions { copy_region };

		cmd_buffer.copyBuffer( this->getVkBuffer(), other.getVkBuffer(), copy_regions );
	}

} // namespace fgl::engine