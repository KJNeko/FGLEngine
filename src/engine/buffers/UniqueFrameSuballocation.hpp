//
// Created by kj16609 on 12/28/23.
//

#pragma once

namespace fgl::engine
{

	//! Wrapper class to allow for easy per-frame suballocation
	template < is_suballocation Suballocation >
	class PerFrameSuballocation
	{
		Buffer& m_buffer;
		std::vector< std::unique_ptr< Suballocation > > m_suballocations {};

	  public:

		PerFrameSuballocation( Buffer& buffer, const std::uint32_t frames_in_flight ) : m_buffer( buffer )
		{
			m_suballocations.reserve( frames_in_flight );
			for ( std::uint16_t i = 0; i < frames_in_flight; ++i )
			{
				static_assert(
					std::is_default_constructible_v< typename Suballocation::value_type >,
					"value_type must be default constructible" );
				m_suballocations.emplace_back( std::make_unique< Suballocation >( buffer ) );
			}
		}

		void setMaxFramesInFlight( std::uint16_t max ) { m_suballocations.resize( max ); }

		Suballocation& operator[]( std::uint16_t index ) { return *m_suballocations[ index ]; }

		void bindForFrame( vk::CommandBuffer& cmd_buffer, std::uint16_t frame_idx )
		{
			m_suballocations[ frame_idx ].bind( cmd_buffer );
		}
	};

} // namespace fgl::engine