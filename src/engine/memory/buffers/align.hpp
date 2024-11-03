//
// Created by kj16609 on 12/28/23.
//

#pragma once

#include <type_traits>

namespace fgl::engine::memory
{

	//! Aligns a memory region to a given alignment
	template < typename T1, typename T2 >
		requires std::is_integral_v< T1 > && std::is_integral_v< T2 >
	constexpr T1 align( const T1 operand, const T2 alignment )
	{
		if ( alignment == 0 || alignment == 1 ) return operand;

		if ( operand == alignment ) return operand;

		if constexpr ( std::same_as< T1, T2 > )
		{
			return ( ( operand + ( alignment - 1 ) ) & ~( alignment - 1 ) );
		}
		else
		{
			return ( ( operand + ( static_cast< T1 >( alignment ) - 1 ) ) & ~( static_cast< T1 >( alignment ) - 1 ) );
		}
	}

	//! Aligns the operand to multiple alignments
	template < typename T1, typename T2, typename... T2s >
	constexpr T1 align( const T1 operand, const T2 alignment, const T2s... alignments )
	{
		return align( align( operand, alignment ), alignments... );
	}

	static_assert( align( 0, 1 ) == 0 );
	static_assert( align( 123, 256 ) == 256 );
	static_assert( align( 256, 256 ) == 256 );
	static_assert( align( 6, 32, 128 ) == 128 );
	static_assert( align( 6, 1, 1, 6 ) == 6 );

} // namespace fgl::engine::memory
