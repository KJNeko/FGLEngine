//
// Created by kj16609 on 6/2/25.
//
#pragma once

#include <format>
#include <vector>

template < typename T >
struct std::formatter< std::vector< T > >
{
	constexpr auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }

	auto format( const std::vector< T >& vec, std::format_context& ctx ) const
	{
		auto out = std::format_to( ctx.out(), "[" );
		bool first = true;
		for ( const auto& item : vec )
		{
			if ( !first )
			{
				out = std::format_to( out, ", " );
			}
			out = std::format_to( out, "{}", item );
			first = false;
		}
		return std::format_to( out, "]" );
	}
};

template <>
struct std::formatter< std::vector< std::string > >;