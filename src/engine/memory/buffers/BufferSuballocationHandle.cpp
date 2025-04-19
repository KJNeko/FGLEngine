//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocationHandle.hpp"

#include "align.hpp"
#include "rendering/CommandBuffers.hpp"
#include "rendering/devices/Device.hpp"

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
		const std::byte* ptr { reinterpret_cast< std::byte* >( &m_parent_buffer->m_buffer ) };
		TracyAllocN( ptr + m_offset, m_size, m_parent_buffer->m_pool_name.c_str() );

		// assert( memory_size != 0 && "BufferSuballocation::BufferSuballocation() called with memory_size == 0" );
	}

	vk::Buffer BufferSuballocationHandle::getVkBuffer() const
	{
		return m_parent_buffer->getVkBuffer();
	}

	void BufferSuballocationHandle::flush() const
	{
		if ( !m_parent_buffer->needsFlush() ) return;
		assert( m_ptr != nullptr && "BufferSuballocationT::flush() called before map()" );

		vk::MappedMemoryRange range {};
		range.memory = m_parent_buffer->getMemory();
		range.offset = getOffset();

		const vk::DeviceSize min_atom_size { Device::getInstance().m_properties.limits.nonCoherentAtomSize };
		const vk::DeviceSize size { m_size };

		assert( size > 0 );

		range.size = align( size, min_atom_size );

		assert( range.size > 0 );

		const std::vector< vk::MappedMemoryRange > ranges { range };

		Device::getInstance()->flushMappedMemoryRanges( ranges );
	}

	BufferSuballocationHandle::BufferSuballocationHandle( const BufferSuballocationHandle& other ) noexcept :
	  m_parent_buffer( other.m_parent_buffer ),
	  m_size( other.m_size ),
	  m_offset( other.m_offset ),
	  m_alignment( other.m_alignment ),
	  m_ptr( other.m_ptr ),
	  m_staged( other.m_staged )
	{}

	BufferSuballocationHandle::~BufferSuballocationHandle()
	{
		const std::byte* ptr { reinterpret_cast< std::byte* >( &m_parent_buffer->m_buffer ) };
		TracyFreeN( ptr + m_offset, m_parent_buffer->m_pool_name.c_str() );
		m_parent_buffer->free( *this );
	}

	vk::BufferCopy BufferSuballocationHandle::copyRegion(
		const BufferSuballocationHandle& target,
		const vk::DeviceSize target_offset,
		const vk::DeviceSize source_offset ) const
	{
		vk::BufferCopy copy {};
		copy.size = std::min( this->m_size, target.m_size );
		copy.srcOffset = this->getOffset() + source_offset;
		copy.dstOffset = target.getOffset() + target_offset;
		return copy;
	}

	void BufferSuballocationHandle::copyTo(
		const vk::raii::CommandBuffer& cmd_buffer,
		const BufferSuballocationHandle& target,
		const std::size_t target_offset ) const
	{
		const vk::BufferCopy copy_region { copyRegion( target, target_offset, m_offset ) };

		const std::vector< vk::BufferCopy > copy_regions { copy_region };

		cmd_buffer.copyBuffer( this->getVkBuffer(), target.getVkBuffer(), copy_regions );
	}

	std::shared_ptr< BufferSuballocationHandle > BufferSuballocationHandle::
		reallocInTarget( const std::shared_ptr< BufferHandle >& new_buffer )
	{
		return new_buffer->allocate( m_size, m_alignment );
	}
} // namespace fgl::engine::memory