//
// Created by kj16609 on 3/17/25.
//
#pragma once

#include "DeviceVector.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine
{
	template < typename T >
	class IndexedVector final : public DeviceVector< T >
	{
		std::queue< std::uint32_t > m_free_indexes {};

		std::uint32_t acquireInternal( const T& t )
		{
			if ( m_free_indexes.empty() )
			{
				this->resize( this->size() + 1 );
				m_free_indexes.push( this->size() - 1 );
			}

			const auto index { m_free_indexes.front() };
			m_free_indexes.pop();

			return index;
		}

	  public:

		DeviceVector< T >& vec() { return static_cast< DeviceVector< T >& >( *this ); }

		class Index
		{
			IndexedVector< T >& m_vector;
			std::uint32_t m_idx { std::numeric_limits< std::uint32_t >::max() };

		  public:

			using GPUValue = std::uint32_t;

			GPUValue idx() const
			{
				FGL_ASSERT( m_idx != std::numeric_limits< std::uint32_t >::max(), "Invalid index" );
				return m_idx;
			}

			using Vec = IndexedVector;

			void update( const T& t ) { return m_vector.updateData( m_idx, t ); }

			Index& operator=( const Index& other ) = delete;

			Index( const Index& other ) : m_vector( other.m_vector ), m_idx( m_vector.acquireInternal() )
			{
				//TODO: Update the data from the original item
			}

			Index& operator=( Index&& other ) = delete;

			Index( Index&& other ) noexcept : m_vector( other.m_vector ), m_idx( other.m_idx )
			{
				other.m_idx = std::numeric_limits< std::uint32_t >::max();
			}

			// Privated to force returning to the IndexedVector
			~Index()
			{
				if ( m_idx == std::numeric_limits< std::uint32_t >::max() ) return;

				log::error( "Index must be returned to the IndexedVector" );
				std::terminate();
			}

		  private:

			Index( IndexedVector< T >& vector, const std::uint32_t idx ) : m_vector( vector ), m_idx( idx ) {}
			friend class IndexedVector;
		};

		IndexedVector() = delete;

		IndexedVector( memory::Buffer& buffer, const std::uint32_t desired_count = 0 ) :
		  DeviceVector< T >( buffer, desired_count )
		{
			for ( std::uint32_t i = 0; i < desired_count; ++i ) m_free_indexes.push( i );
		}

		Index acquire( const T& t )
		{
			if ( m_free_indexes.empty() )
			{
				this->resize( this->size() + 1 );
				m_free_indexes.push( this->size() - 1 );
			}

			Index index { *this, m_free_indexes.front() };
			m_free_indexes.pop();

			index.update( t );

			return index;
		}

		void release( Index&& index_i )
		{
			Index index { std::forward< Index >( index_i ) };
			m_free_indexes.push( index.m_idx );
			index.m_idx = std::numeric_limits< std::uint32_t >::max();
		}
	};

} // namespace fgl::engine