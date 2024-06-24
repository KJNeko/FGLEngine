//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "BufferVector.hpp"
#include "HostVector.hpp"
#include "engine/logging/logging.hpp"

namespace fgl::engine
{

	template < typename T >
	class DeviceVector final : public BufferVector
	{
		std::unique_ptr< HostVector< T > > m_staging_buffer {};
		bool staged { false };

	  public:

		DeviceVector( Buffer& buffer, const std::uint32_t count = 1 ) : BufferVector( buffer, count, sizeof( T ) )
		{
			log::debug( "Creating DeviceVector of size {}", count );
			assert( count != 0 && "BufferSuballocationVector::BufferSuballocationVector() called with count == 0" );
		}

		bool hasStaging() const { return m_staging_buffer != nullptr; }

		void createStaging( const std::vector< T >& data )
		{
			m_staging_buffer = std::make_unique< HostVector< T > >( getGlobalStagingBuffer(), data );
		}

		HostVector< T >& getStaging() { return *m_staging_buffer; }

		void stage()
		{
			auto buffer { Device::getInstance().beginSingleTimeCommands() };

			stage( buffer );

			Device::getInstance().endSingleTimeCommands( buffer );

			dropStaging();
		}

		void stage( vk::raii::CommandBuffer& command_buffer )
		{
			assert( m_staging_buffer && "DeviceVector::stage() called without staging buffer" );

			//Copy
			vk::BufferCopy copy_region { m_staging_buffer->getOffset(), this->m_offset, this->m_byte_size };

			command_buffer.copyBuffer( m_staging_buffer->getVkBuffer(), this->getVkBuffer(), copy_region );
			staged = true;
		}

		void dropStaging()
		{
			assert( staged && "Staging buffer has not been commanded to write yet!" );
			m_staging_buffer.reset();
		}

		/**
		 * @brief Constructs a new DeviceVector from a vector, Requires a command buffer to copy the data to the device
		 * @param buffer
		 * @param data
		 * @param command_buffer
		 */
		DeviceVector( Buffer& buffer, const std::vector< T >& data ) :
		  DeviceVector( buffer, static_cast< std::uint32_t >( data.size() ) )
		{
			createStaging( data );
		}
	};

} // namespace fgl::engine