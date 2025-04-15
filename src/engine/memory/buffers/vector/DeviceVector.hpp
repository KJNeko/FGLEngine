//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "BufferVector.hpp"
#include "concepts.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::memory
{
	struct Buffer;
}

namespace fgl::engine
{

	template < typename T >
	class DeviceVector : public memory::BufferVector, public memory::DeviceVectorBase
	{
	  public:

		using Type = T;

		DeviceVector( memory::Buffer& buffer, const std::uint32_t count = 0 ) :
		  BufferVector( buffer, count, sizeof( T ) )
		{
			// const auto size_str { literals::size_literals::toString( count * sizeof( T ) ) };
			// assert( count != 0 && "BufferSuballocationVector::BufferSuballocationVector() called with count == 0" );
		}

		FGL_DELETE_DEFAULT_CTOR( DeviceVector );
		FGL_DELETE_COPY( DeviceVector );
		FGL_DEFAULT_MOVE( DeviceVector );

		/**
		 * @brief Constructs a new DeviceVector from a vector using an allocation of the supplied buffer
		 * @param buffer buffer to allocate from
		 * @param data
		 */
		DeviceVector( memory::Buffer& buffer, const std::vector< T >& data ) :
		  DeviceVector( buffer, static_cast< std::uint32_t >( data.size() ) )
		{
			memory::TransferManager::getInstance().copyToVector< T, DeviceVector< T > >( data, *this );
		}

		// void resize( const std::size_t new_size ) { BufferVector::resize( new_size ); }
		// void resizeDiscard( const uint32_t size ) { BufferVector::resizeDiscard( new_size ); }

		void updateData( const std::size_t idx, const T& data )
		{
			assert( idx < m_count );
			memory::TransferManager::getInstance().copyToVector< T, DeviceVector< T > >( data, idx, *this );
		}
	};

} // namespace fgl::engine