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
		  BufferSuballocation( buffer.suballocate( count * stride ) ),
		  m_count( count ),
		  m_stride( stride )
		{}

		BufferVector( const BufferVector& ) = delete;
		BufferVector( BufferVector&& ) = default;

		BufferVector& operator=( BufferVector&& other )
		{
			m_count = other.m_count;
			m_stride = other.m_stride;

			BufferSuballocation::operator=( std::move( other ) );

			return *this;
		}

		BufferVector& operator=( const BufferVector& ) = delete;

	  public:

		//! Returns the offset count from the start of the buffer to the first element
		[[nodiscard]] std::uint32_t getOffsetCount() const
		{
			return static_cast< std::uint32_t >( this->m_offset / m_stride );
		}

		[[nodiscard]] std::uint32_t count() const noexcept { return m_count; }

		[[nodiscard]] std::uint32_t stride() const noexcept { return m_stride; }

		void resize( const std::uint32_t count )
		{
			BufferVector other { this->getBuffer(), count, m_stride };

			Device::getInstance().copyBuffer( this->getBuffer(), other.getBuffer(), 0, 0, this->size() );

			*this = std::move( other );
		}
	};

} // namespace fgl::engine
