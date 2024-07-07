//
// Created by kj16609 on 11/29/23.
//

#pragma once

#include <algorithm>

namespace fgl::engine
{

	template < typename T, typename... Ts >
	void hashCombine( std::size_t& seed, const T& v, const Ts&... ts )
	{
		std::hash< T > hasher;
		seed ^= hasher( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );

		( hashCombine( seed, ts ), ... );
	}

} // namespace fgl::engine