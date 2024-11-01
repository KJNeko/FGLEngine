//
// Created by kj16609 on 6/5/24.
//

#include "filesystem.hpp"

auto format_ns::formatter< std::filesystem::path >::format( const std::filesystem::path& path, format_context& ctx )
	const -> decltype( ctx.out() )
{
	if ( print_canonical && std::filesystem::exists( path ) )
	{
		if ( print_exists )
			return format_ns::format_to(
				ctx.out(),
				"[\"{}\", (Canonical: \"{}\") Exists: \"{}\"]",
				path.string(),
				std::filesystem::canonical( path ).string(),
				std::filesystem::exists( path ) ? "True" : "False" );
		else
			return format_ns::format_to(
				ctx.out(), "[\"{}\" (Canonical: \"{}\")]", path.string(), std::filesystem::canonical( path ).string() );
	}
	else
	{
		if ( print_exists )
			return format_ns::
				format_to( ctx.out(), "[\"{}\"]", path.string(), std::filesystem::exists( path ) ? "True" : "False" );
		else
			return format_ns::format_to( ctx.out(), "[\"{}\"]", path.string() );
	}
}

auto format_ns::formatter< std::source_location >::format( const std::source_location& loc, format_context& ctx )
	-> decltype( ctx.out() )
{
	return format_ns::
		format_to( ctx.out(), "File: {}:{}\n\tFunction: {}\n\t", loc.file_name(), loc.line(), loc.function_name() );
}
