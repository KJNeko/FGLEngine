//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "engine/buffers/BufferSuballocation.hpp"

namespace fgl::engine
{
	//! Number of spares to allocate when resizing beyond the current capacity + current spare
	constexpr std::uint32_t SPARE_ALLOCATION_COUNT { 16 };

	class BufferVector : public BufferSuballocation
	{
	  protected:

		//! Number of items in the vector
		std::uint32_t m_count { std::numeric_limits< std::uint32_t >::quiet_NaN() };

		//! Bytes for each item
		std::uint32_t m_stride { std::numeric_limits< std::uint32_t >::quiet_NaN() };

		//TODO: Implement spare
		//std::uint32_t m_spare_count { 0 };

		BufferVector() = delete;

		BufferVector( Buffer& buffer, std::uint32_t count, std::uint32_t stride ) :
		  BufferSuballocation( buffer.suballocate( count * stride ) ),
		  m_count( count ),
		  m_stride( stride )
		{}

		BufferVector( const BufferVector& ) = delete;

		BufferVector& operator=( const BufferVector& ) = delete;

		BufferVector( BufferVector&& ) = default;

		BufferVector& operator=( BufferVector&& other ) = default;

	  public:

		//! Returns the total byte capacity of the buffer
		[[nodiscard]] std::size_t byteCapacity() const noexcept { return m_count * m_stride; }

		//! Returns the offset count from the start of the buffer to the first element
		[[nodiscard]] std::uint32_t getOffsetCount() const
		{
			assert( !std::isnan( m_count ) );
			assert( !std::isnan( m_stride ) );
			assert( m_offset % m_stride == 0 && "Offset must be aligned from the stride" );

			return static_cast< std::uint32_t >( this->m_offset / m_stride );
		}

		[[nodiscard]] std::uint32_t stride() const noexcept
		{
			assert( !std::isnan( m_stride ) );
			return m_stride;
		}

		[[nodiscard]] std::uint32_t size() const noexcept { return m_count; }

		void resize( const std::uint32_t count )
		{
			assert( count > 0 );
			assert( !std::isnan( m_stride ) );
			assert( !std::isnan( m_count ) );

			//If the capacity is higher then what we are requesting then we simply just ignore the request.
			// TODO: Maybe this is bad? I'm unsure. But reducing the number of allocations is always good
			if ( count < m_count ) return;

			BufferVector other { this->getBuffer(), count, m_stride };

			Device::getInstance().copyBuffer( this->getBuffer(), other.getBuffer(), 0, 0, this->size() );

			*this = std::move( other );
		}
	};

} // namespace fgl::engine
