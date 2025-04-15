//
// Created by kj16609 on 12/28/23.
//

#pragma once

#include "engine/concepts/is_suballocation.hpp"

namespace fgl::engine
{

	//! Wrapper class to allow for easy per-frame suballocation
	template < memory::is_suballocation Suballocation >
	class PerFrameSuballocation
	{
		std::vector< std::unique_ptr< Suballocation > > m_suballocations {};

	  public:

		explicit PerFrameSuballocation( memory::Buffer& buffer )
		{
			m_suballocations.reserve( constants::MAX_FRAMES_IN_FLIGHT );

			for ( std::uint16_t i = 0; i < constants::MAX_FRAMES_IN_FLIGHT; ++i )
			{
				static_assert(
					std::is_default_constructible_v< typename Suballocation::value_type >,
					"value_type must be default constructible" );
				m_suballocations.emplace_back( std::make_unique< Suballocation >( buffer ) );
			}
		}

		// void setMaxFramesInFlight( std::uint16_t max ) { m_suballocations.resize( max ); }

		Suballocation& operator[]( std::size_t index )
		{
			assert( m_suballocations.size() >= index );
			assert( m_suballocations[ index ] );

			return *m_suballocations[ index ];
		}
	};

} // namespace fgl::engine