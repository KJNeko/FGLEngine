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
				std::memcpy( this->ptr(), vec.data(), this->count() * sizeof( T ) );
			}
			else
				assert( "Stride must be equal to sizeof(T)" );

			this->flush();
		}

		void flush() { this->flushRange( 0, this->count() ); }

		void flushRange( const std::uint32_t start_idx, const std::uint32_t end_idx )
		{
			if ( this->m_count == 0 ) [[unlikely]]
				return;

			assert(
				start_idx < this->m_count && "BufferSuballocationVector::flushRange start_idx index out of bounds" );
			assert( end_idx <= this->m_count && "BufferSuballocationVector::flushRange end_idx index out of bounds" );
			assert(
				start_idx < end_idx
				&& "BufferSuballocationVector::flushRange start_idx index must be less than end_idx index" );
			assert(
				( end_idx - start_idx ) != 0
				&& "BufferSuballocationVector::flushRange end_idx must be at least +1 from start_idx" );

			const auto count { end_idx - start_idx };
			assert( count > 0 && "Count must be larger then 0" );

			BufferSuballocation::flush( start_idx * this->m_stride, count * this->m_stride );
		}

		HostVector& operator=( const std::vector< T >& vec )
		{
			assert( this->m_count == vec.size() && "HostVector::operator=() called with vector of different size" );

			if ( this->m_stride == sizeof( T ) )
			{
				std::memcpy( this->ptr(), vec.data(), this->count() * sizeof( T ) );
			}
			else
				assert( "Stride must be equal to sizeof(T)" );

			this->flush();

			return *this;
		}

		void push_back( const T& t )
		{
			resize( this->m_count + 1 );

			*this[ this->m_count++ ] = t;

			//--this->m_spare_count;
		}

		~HostVector() {}
	};

} // namespace fgl::engine