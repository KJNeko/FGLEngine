//
// Created by kj16609 on 7/2/24.
//

#pragma once

#include "BufferSuballocation.hpp"

namespace fgl::engine
{
	namespace memory
	{
		struct Buffer;
	}

	//! Single element allocation of T
	template < typename T >
	struct HostSingleT final : public memory::BufferSuballocation
	{
		friend class TransferData;

		using value_type = T;

		HostSingleT() = delete;
		HostSingleT( const HostSingleT& ) = delete;
		HostSingleT( HostSingleT&& ) = delete;
		HostSingleT& operator=( const HostSingleT& ) = delete;

		HostSingleT( const memory::Buffer& buffer ) : memory::BufferSuballocation( buffer, sizeof( T ), alignof( T ) ) {}

		HostSingleT& operator=( T& t )
		{
			FGL_ASSERT( this->ptr(), "Invalid host pointer for HostSingleT!" );
			*static_cast< T* >( this->ptr() ) = t;

			flush();

			return *this;
		}

		void flush() { BufferSuballocation::flush( 0, this->m_byte_size ); }
	};

} // namespace fgl::engine
