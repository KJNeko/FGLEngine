//
// Created by kj16609 on 12/28/23.
//

#pragma once

#include <iostream>
#include <type_traits>

#include "FGL_DEFINES.hpp"

namespace fgl::engine::memory
{

	template < typename T1 >
	constexpr T1 combineAlignment( T1 first )
	{
		return first;
	}

	template < typename T1, typename T2 >
	constexpr T1 combineAlignment( T1 first, T2 second )
	{
		return std::max( first, static_cast< T1 >( second ) );
	}

	template < typename T1, typename... Ts >
		requires( sizeof...( Ts ) > 0 )
	constexpr T1 combineAlignment( T1 first, Ts... extras )
	{
		return combineAlignment( first, combineAlignment( extras... ) );
	}

	//! Aligns a memory region to a given alignment
	template < typename T1, typename T2 >
		requires std::is_integral_v< T1 > && std::is_integral_v< T2 >
	constexpr T1 align( const T1 operand, const T2 alignment )
	{
		if ( alignment == 0 || alignment == 1 ) return operand;

		const auto offset { operand % alignment };

		if ( offset == 0 ) return operand;

		// only go forward, going back might make us back into an already allocated area
		return operand + ( alignment - offset );

		// bitwise alignments only work for 2^x
		/*
		if constexpr ( std::same_as< T1, T2 > )
		{
			return ( ( operand + alignment - 1 ) & ~( alignment - 1 ) );
		}
		else
		{
			return ( ( operand + static_cast< T1 >( alignment ) - 1 ) & ~( static_cast< T1 >( alignment ) - 1 ) );
		}
		*/
	}

	//! Aligns the operand to multiple alignments
	template < typename T1, typename... T2s >
	constexpr T1 align( const T1 operand, const T2s... alignments )
	{
		const auto true_alignment { combineAlignment( alignments... ) };

		const auto alignment { align( operand, true_alignment ) };

		return alignment;
	}

	static_assert( align( 0, 1 ) == 0 );
	static_assert( align( 123, 256 ) == 256 );
	static_assert( align( 256, 256 ) == 256 );
	static_assert( align( 6, 32, 128 ) == 128 );
	static_assert( align( 6, 1, 1, 6 ) == 6 );
	static_assert( combineAlignment( 44, 4 ) == 44 );
	static_assert( align( 123, 44, 4 ) % 44 == 0 );
	static_assert( align( 278528, 44 ) % 44 == 0 );
	static_assert( align( 278528, 4 ) % 4 == 0 );
	static_assert( align( 278529, 4 ) == 278528 + 4 );
	static_assert( align( 278528, 4 ) == 278528 );
	static_assert( align( 278527, 4 ) == 278528 );
	static_assert( align( 278528, 3 ) % 3 == 0 );

} // namespace fgl::engine::memory
