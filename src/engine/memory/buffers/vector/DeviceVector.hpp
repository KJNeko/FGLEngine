//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "BufferVector.hpp"
#include "concepts.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine
{
	namespace memory
	{
		class Buffer;
	}
}

namespace fgl::engine
{
	template < typename T >
	class DeviceVector final : public memory::BufferVector, public memory::DeviceVectorBase
	{
	  public:

		DeviceVector( memory::Buffer& buffer, const std::uint32_t count = 1 ) : BufferVector( buffer, count, sizeof( T ) )
		{
			log::debug(
				"Creating DeviceVector of size {}", fgl::literals::size_literals::to_string( count * sizeof( T ) ) );
			assert( count != 0 && "BufferSuballocationVector::BufferSuballocationVector() called with count == 0" );
		}

		/**
		 * @brief Constructs a new DeviceVector from a vector using an allocation of the supplied buffer
		 * @param buffer buffer to allocate from
		 * @param data
		 */
		DeviceVector( memory::Buffer& buffer, const std::vector< T >& data ) :
		  DeviceVector( buffer, static_cast< std::uint32_t >( data.size() ) )
		{
			memory::TransferManager::getInstance().copyToVector( data, *this );
		}
	};

} // namespace fgl::engine