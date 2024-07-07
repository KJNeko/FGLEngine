//
// Created by kj16609 on 1/25/24.
//

#pragma once

#include <numeric>

template < typename T, std::uint64_t max_count = 100 >
	requires std::is_integral_v< T > || std::is_floating_point_v< T >
class Average
{
	std::once_flag flag {};
	std::array< T, max_count > data {};

  public:

	consteval std::uint64_t count() const { return max_count; }

	FGL_FLATTEN void push( const T t )
	{
		std::call_once( flag, [ this, t ]() { std::fill( data.begin(), data.end(), t ); } );
		std::shift_right( data.begin(), data.end(), 1 );
		data[ 0 ] = t;
	}

	T average() const
	{
		const T accum { std::accumulate( data.begin(), data.end(), T() ) };

		return accum / static_cast< T >( data.size() );
	}
};
