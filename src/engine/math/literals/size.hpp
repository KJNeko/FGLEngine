//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include <cstdint>

namespace fgl::literals::size_literals
{

	constexpr unsigned long long int operator""_B( const unsigned long long int size )
	{
		return size;
	}

	constexpr unsigned long long int operator""_KiB( const unsigned long long int size )
	{
		return size * 1024;
	}

	constexpr unsigned long long int operator""_MiB( const unsigned long long int size )
	{
		return size * 1024_KiB;
	}

	constexpr unsigned long long int operator""_GiB( const unsigned long long int size )
	{
		return size * 1024_MiB;
	}

	inline std::string toString( const unsigned long long int size )
	{
		if ( size < 1024_B ) return std::to_string( size ) + " B";
		if ( size < 1024_KiB ) return std::to_string( size / 1024 ) + " KiB";
		if ( size < 1024_MiB ) return std::to_string( size / 1024_KiB ) + " MiB";
		if ( size < 1024_GiB ) return std::to_string( size / 1024_MiB ) + " GiB";
		return std::to_string( size / 1024_GiB ) + " TiB";
	}

} // namespace fgl::literals::size_literals