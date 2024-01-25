//
// Created by kj16609 on 1/25/24.
//

#pragma once

#include <concepts>
#include <numeric>

template < typename T, std::uint64_t max_count = 100 >
	requires std::is_integral_v< T > || std::is_floating_point_v< T >
class Average
{
	std::array< T, max_count > data {};

  public:

	std::array< T, max_count >& getData() { return data; }

	consteval std::uint64_t count() const { return max_count; }

	void push( const T t )
	{
		std::array< T, max_count > shift_array {};
		for ( std::uint64_t i = 1; i < max_count; ++i )
		{
			shift_array[ i - 1 ] = data[ i ];
		}

		shift_array[ max_count - 1 ] = t;
		data = shift_array;
	}

	T average() const
	{
		const T accum { std::accumulate( data.begin(), data.end(), T() ) };

		return accum / static_cast< T >( data.size() );
	}
};
