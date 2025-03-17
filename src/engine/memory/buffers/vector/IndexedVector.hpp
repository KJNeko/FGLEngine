//
// Created by kj16609 on 3/17/25.
//
#pragma once

#include "DeviceVector.hpp"

namespace fgl::engine
{
	template < typename T >
	class IndexedVector : DeviceVector< T >
	{
		std::queue< std::uint32_t > m_free_indexes {};

	  public:

		class Index
		{
			IndexedVector< T >& m_vector;
			std::uint32_t m_idx;

		  public:

			void update( const T& t )
			{
				memory::TransferManager::getInstance()
					.copyToVector< T, DeviceVector< T > >( m_vector.m_data, m_idx, t );
			}

		  private:

			Index( IndexedVector< T >& vector, const std::uint32_t idx ) : m_vector( vector ), m_idx( idx ) {}

			// Privated to force returning to the IndexedVector
			~Index() = default;
		};

		IndexedVector() = delete;

		IndexedVector( memory::Buffer& buffer, const std::uint32_t desired_count = 0 ) :
		  DeviceVector< T >( buffer, desired_count )
		{
			for ( std::uint32_t i = 0; i < desired_count; ++i ) m_free_indexes.push( i );
		}

		Index< T > acquire( const T& t )
		{
			if ( m_free_indexes.empty() )
			{
				resize( this->size() + 1 );
				m_free_indexes.push( this->size() - 1 );
			}

			Index< T > index { *this, m_free_indexes.front() };
			m_free_indexes.pop();

			index.update( t );

			return index;
		}

		void release( Index< T >&& index_i )
		{
			Index< T > index { std::forward< Index< T > >( index_i ) };
			m_free_indexes.push( index.m_idx );
		}
	};

} // namespace fgl::engine