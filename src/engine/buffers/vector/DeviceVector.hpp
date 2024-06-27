//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "BufferVector.hpp"
#include "concepts.hpp"
#include "engine/assets/TransferManager.hpp"
#include "engine/literals/size.hpp"
#include "engine/logging/logging.hpp"

namespace fgl::engine
{
	template < typename T >
	class DeviceVector final : public BufferVector, public DeviceVectorBase
	{
	  public:

		DeviceVector( Buffer& buffer, const std::uint32_t count = 1 ) : BufferVector( buffer, count, sizeof( T ) )
		{
			log::debug(
				"Creating DeviceVector of size {}", fgl::literals::size_literals::to_string( count * sizeof( T ) ) );
			assert( count != 0 && "BufferSuballocationVector::BufferSuballocationVector() called with count == 0" );
		}

		/**
		 * @brief Constructs a new DeviceVector from a vector using an allocation of the supplied buffer
		 * @param buffer buffer to suballocate from
		 * @param data
		 */
		DeviceVector( Buffer& buffer, const std::vector< T >& data ) :
		  DeviceVector( buffer, static_cast< std::uint32_t >( data.size() ) )
		{
			TransferManager::getInstance().copyToBuffer( data, *this );
		}
	};

} // namespace fgl::engine