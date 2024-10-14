//
// Created by kj16609 on 6/26/24.
//

#include "BufferVector.hpp"

#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/memory/buffers/Buffer.hpp"

namespace fgl::engine::memory
{

	[[nodiscard]] BufferVector::BufferVector( Buffer& buffer, std::uint32_t count, std::uint32_t stride ) :
	  BufferSuballocation( buffer.allocate( count * stride ) ),
	  m_count( count ),
	  m_stride( stride )
	{}

	//! Returns the offset count from the start of the buffer to the first element
	[[nodiscard]] std::uint32_t BufferVector::getOffsetCount() const
	{
		assert( !std::isnan( m_count ) );
		assert( !std::isnan( m_stride ) );
		assert( m_count * m_stride == this->bytesize() );
		assert( m_offset % m_stride == 0 && "Offset must be aligned from the stride" );

		return static_cast< std::uint32_t >( this->m_offset / m_stride );
	}

	[[nodiscard]] std::uint32_t BufferVector::stride() const noexcept
	{
		assert( !std::isnan( m_stride ) );
		assert( m_count * m_stride <= this->bytesize() );
		return m_stride;
	}

	[[nodiscard]] std::uint32_t BufferVector::size() const noexcept
	{
		assert( !std::isnan( m_count ) );
		assert( m_count * m_stride <= this->bytesize() );
		return m_count;
	}

	void BufferVector::resize( const std::uint32_t count )
	{
		assert( count > 0 );
		assert( !std::isnan( m_stride ) );
		assert( !std::isnan( m_count ) );

		//If the capacity is higher then what we are requesting then we simply just ignore the request.
		// TODO: Maybe this is bad? I'm unsure. But reducing the number of allocations is always good
		if ( count < m_count ) return;

		BufferVector other { this->getBuffer(), count, m_stride };

		TransferManager::getInstance().copyToVector( *this, other, 0 );

		*this = std::move( other );
	}

} // namespace fgl::engine::memory