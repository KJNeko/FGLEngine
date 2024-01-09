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

		std::uint32_t m_count;
		std::uint32_t m_stride;

		//TODO: Implement spare
		//std::uint32_t m_spare_count { 0 };

		BufferVector() = delete;

		BufferVector( Buffer& buffer, std::uint32_t count, std::uint32_t stride ) :
		  BufferSuballocation( buffer, count * stride, 1 ),
		  m_count( count ),
		  m_stride( stride )
		{}

		BufferVector( const BufferVector& ) = delete;
		BufferVector( BufferVector&& ) = delete;

		BufferVector& operator=( BufferVector&& other )
		{
			this->m_buffer.free( this->m_info );
			this->m_info = other.m_info;
			return *this;
		}

		BufferVector& operator=( const BufferVector& ) = delete;

	  public:

		//! Returns the offset count from the start of the buffer to the first element
		[[nodiscard]] std::uint32_t getOffsetCount()
		{
			return static_cast< std::uint32_t >( this->m_info.offset / m_stride );
		}

		[[nodiscard]] std::uint32_t count() const noexcept { return m_count; }

		[[nodiscard]] std::uint32_t stride() const noexcept { return m_stride; }

		void resize( const std::uint32_t count )
		{
			BufferVector other { this->m_buffer, count, m_stride };

			Device::getInstance().copyBuffer( this->m_buffer, other.m_buffer, 0, 0, this->size() );

			*this = std::move( other );
		}
	};

} // namespace fgl::engine
