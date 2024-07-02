//
// Created by kj16609 on 7/2/24.
//

#pragma once

namespace fgl::engine
{

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

		HostSingleT( memory::Buffer& buffer ) :
		  memory::BufferSuballocation( buffer.suballocate( sizeof( T ), alignof( T ) ) )
		{}

		HostSingleT& operator=( T& t )
		{
			*static_cast< T* >( this->ptr() ) = t;

			flush();

			return *this;
		}

		void flush() { BufferSuballocation::flush( 0, this->m_byte_size ); }
	};

} // namespace fgl::engine
