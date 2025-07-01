//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocationHandle.hpp"

#include "BufferHandle.hpp"
#include "BufferSuballocation.hpp"
#include "assets/transfer/TransferManager.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::memory
{
	vk::Buffer BufferSuballocationHandle::getBuffer() const
	{
		return m_parent_buffer->getVkBuffer();
	}

	BufferSuballocationHandle::BufferSuballocationHandle(
		const Buffer& p_buffer,
		const vk::DeviceSize offset,
		const vk::DeviceSize memory_size,
		const vk::DeviceSize alignment ) :
	  m_parent_buffer( p_buffer ),
	  m_size( memory_size ),
	  m_offset( offset ),
	  m_alignment( alignment ),
	  m_ptr( m_parent_buffer->map( *this ) )
	{
		// assert( memory_size != 0 && "BufferSuballocation::BufferSuballocation() called with memory_size == 0" );
	}

	vk::Buffer BufferSuballocationHandle::getVkBuffer() const
	{
		return m_parent_buffer->getVkBuffer();
	}

	BufferSuballocationHandle::~BufferSuballocationHandle()
	{
		m_parent_buffer->free( *this );
	}

	vk::BufferCopy BufferSuballocationHandle::
		copyRegion( const BufferSuballocationHandle& target, const std::size_t suballocation_offset ) const
	{
		vk::BufferCopy copy {};
		copy.size = std::min( this->m_size, target.m_size );
		copy.srcOffset = this->getOffset();
		copy.dstOffset = target.getOffset() + suballocation_offset;
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