//
// Created by kj16609 on 12/28/23.
//

#pragma once

#include "BufferVector.hpp"

namespace fgl::engine
{

	template < typename T >
	class DeviceVector;

	/**
	 * A vector device with the ability to flush to the device.
	 * @tparam T
	 * @tparam Buffer
	 */
	template < typename T >
	class HostVector final : public BufferVector
	{
	  public:

		using value_type = T;

		HostVector() = delete;
		HostVector( const HostVector& ) = delete;
		HostVector& operator=( const HostVector& ) = delete;
		HostVector& operator=( HostVector&& ) = delete;
		HostVector( HostVector&& other ) = delete;

		HostVector( Buffer& buffer, const std::uint32_t count = 1 ) : BufferVector( buffer, count, sizeof( T ) ) {}

		HostVector( Buffer& buffer, const std::vector< T >& vec ) :
		  HostVector( buffer, static_cast< std::uint32_t >( vec.size() ) )
		{
			if ( this->m_stride == sizeof( T ) )
			{
				std::memcpy( this->ptr(), vec.data(), vec.size() * sizeof( T ) );
			}
			else
				assert( "Stride must be equal to sizeof(T)" );

			this->flush();
		}

		void flush() { this->flushRange( 0, size() ); }

		void flushRange( const std::uint32_t start_idx, const std::uint32_t end_idx )
		{
			assert(
				start_idx < this->m_count && "BufferSuballocationVector::flushRange start_idx index out of bounds" );
			assert( end_idx <= this->m_count && "BufferSuballocationVector::flushRange end_idx index out of bounds" );
			assert(
				end_idx - start_idx > 0
				&& "BufferSuballocationVector::flushRange: end_idx - start_idx must be higher then 0" );
			assert(
				start_idx < end_idx
				&& "BufferSuballocationVector::flushRange start_idx index must be less than end_idx index" );
			assert(
				( end_idx - start_idx ) != 0
				&& "BufferSuballocationVector::flushRange end_idx must be at least +1 from start_idx" );

			[[maybe_unused]] const auto count { end_idx - start_idx };
			assert( count > 0 && "Count must be larger then 0" );
			assert( count <= m_count );

			BufferSuballocation::flush( start_idx * this->m_stride, end_idx * this->m_stride );
		}

		HostVector& operator=( const std::vector< T >& vec )
		{
			if ( this->m_stride == sizeof( T ) )
			{
				std::memcpy( this->ptr(), vec.data(), vec.size() * sizeof( T ) );
			}
			else
				assert( "Stride must be equal to sizeof(T)" );

			this->flush();

			return *this;
		}

		~HostVector() {}
	};

} // namespace fgl::engine