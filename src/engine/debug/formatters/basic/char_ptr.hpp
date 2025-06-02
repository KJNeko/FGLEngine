//
// Created by kj16609 on 6/2/25.
//
#pragma once

#include <format>
#include <string_view>

template <>
struct std::formatter< const char* >
{
	constexpr auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }

	auto format( const char* value, std::format_context& ctx ) const
	{
		if ( !value )
			return std::format_to( ctx.out(), "(null)" );
		else
			return std::format_to< std::string_view >( ctx.out(), "{}", std::string_view( value ) );
	}
};