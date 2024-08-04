//
// Created by kj16609 on 6/5/24.
//

#pragma once

#include <filesystem>
#include <format>
#include <source_location>

namespace format_ns = std;

template <>
struct format_ns::formatter< std::filesystem::path >
{
	bool print_canonical { false };
	bool print_exists { false };

	constexpr format_parse_context::iterator parse( format_parse_context& ctx )
	{
		//Check if ctx has 'c' 'ce' or 'e' and return the itterator after it
		auto idx { ctx.begin() };
		const auto end { ctx.end() };

		if ( idx != end && *idx == 'c' )
		{
			print_canonical = true;
			++idx;
		}

		if ( idx != end && *idx == 'e' )
		{
			print_exists = true;
			++idx;
		}

		return idx;
	}

	format_context::iterator format( const std::filesystem::path& path, format_context& ctx ) const;
};

template <>
struct format_ns::formatter< std::source_location >
{
	constexpr format_parse_context::iterator parse( format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const std::source_location& loc, format_context& ctx ) const;
};

template <>
struct format_ns::formatter< format_ns::format_string<> >
{
	constexpr format_parse_context::iterator parse( format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const format_ns::format_string<>& str, format_context& ctx ) const;
};
