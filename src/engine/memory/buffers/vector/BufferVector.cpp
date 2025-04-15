//
// Created by kj16609 on 6/26/24.
//

#include "BufferVector.hpp"

#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/memory/buffers/BufferHandle.hpp"
#include "memory/buffers/BufferHandle.hpp"

namespace fgl::engine::memory
{

	constexpr std::uint32_t min_capacity { 1024 };

	[[nodiscard]] BufferVector::BufferVector( Buffer& buffer, std::uint32_t count, std::uint32_t stride ) :
	  BufferSuballocation( buffer->allocate( std::max( count, min_capacity ) * stride, stride ) ),
	  m_count( count ),
	  m_capacity( std::max( count, min_capacity ) ),
	  m_stride( stride )
	{
		FGL_ASSERT( m_stride > 0, "Stride was not greater then zero" );
		FGL_ASSERT(
			m_offset % m_stride == 0,
			std::format(
				"{} % {} != 0 (Was {}). The offset should be aligned with the stride",
				m_offset,
				m_stride,
				m_offset % m_stride ) );
	}

	//! Returns the offset count from the start of the buffer to the first element
	[[nodiscard]] std::uint32_t BufferVector::getOffsetCount() const
	{
		assert( !std::isnan( m_count ) );
		assert( !std::isnan( m_stride ) );
		assert( m_capacity * m_stride == this->bytesize() );
		FGL_ASSERT(
			m_offset % m_stride == 0, std::format( "{} % {} != 0 (Was {})", m_offset, m_stride, m_offset % m_stride ) );

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

	[[nodiscard]] std::uint32_t BufferVector::capacity() const noexcept
	{
		assert( !std::isnan( m_count ) );
		assert( m_count * m_stride <= this->bytesize() );
		return m_capacity;
	}

	void BufferVector::resize( const std::uint32_t count )
	{
		assert( !std::isnan( m_stride ) );
		assert( !std::isnan( m_count ) );

		// we are reclaiming size
		//TODO: Figure out a way to truely reclaim any size
		if ( count < capacity() )
		{
			m_count = count;
			return;
		}

		// the capacity is not enough for the new size, we must reallocate.
		if ( count > capacity() )
		{
			BufferVector other { this->getBuffer(), count, m_stride };

			TransferManager::getInstance().copyToVector( *this, other, 0 );

			*this = std::move( other );
		}

		this->m_count = count;
	}

	void BufferVector::resizeDiscard( const std::uint32_t count )
	{
		assert( count > 0 );
		assert( !std::isnan( m_stride ) );
		assert( !std::isnan( m_count ) );

		// we are reclaiming size
		//TODO: Figure out a way to truely reclaim any size
		if ( count < capacity() )
		{
			m_count = count;
			return;
		}

		// the capacity is not enough for the new size, we must reallocate.
		if ( count > capacity() )
		{
			BufferVector other { this->getBuffer(), count, m_stride };

			// discard doesn't want the old data, so we can safely just discard it
			// TransferManager::getInstance().copyToVector( *this, other, 0 );

			*this = std::move( other );
		}

		this->m_count = count;
	}

} // namespace fgl::engine::memory