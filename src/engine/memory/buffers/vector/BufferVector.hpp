//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "engine/memory/buffers/BufferSuballocation.hpp"

namespace fgl::engine::memory
{
	class Buffer;

	//! Number of spares to allocate when resizing beyond the current capacity + current spare
	constexpr std::uint32_t SPARE_ALLOCATION_COUNT { 16 };

	class BufferVector : public BufferSuballocation
	{
	  protected:

		//! Number of items in the vector
		std::uint32_t m_count { std::numeric_limits< std::uint32_t >::quiet_NaN() };
		std::uint32_t m_capacity { std::numeric_limits< std::uint32_t >::quiet_NaN() };

		//! Bytes for each item
		std::uint32_t m_stride { std::numeric_limits< std::uint32_t >::quiet_NaN() };

		//TODO: Implement spare
		//std::uint32_t m_spare_count { 0 };

		BufferVector() = delete;

		BufferVector( Buffer& buffer, std::uint32_t count, std::uint32_t stride );

		BufferVector( const BufferVector& ) = delete;

		BufferVector& operator=( const BufferVector& ) = delete;

		BufferVector( BufferVector&& ) = default;

		BufferVector& operator=( BufferVector&& other ) = default;

	  public:

		std::uint32_t getOffsetCount() const;
		std::uint32_t stride() const noexcept;
		std::uint32_t size() const noexcept;
		std::uint32_t capacity() const noexcept;
		void resize( std::uint32_t count );
		void resizeDiscard( std::uint32_t count );
		// void reserve( std::uint32_t count );
	};

} // namespace fgl::engine::memory
