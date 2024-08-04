//
// Created by kj16609 on 8/2/24.
//

#pragma once

#include <glm/glm.hpp>

#include <format>

template <>
struct std::formatter< glm::mat4 >
{
	constexpr format_parse_context::iterator parse( std::format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const glm::mat4& mat4, format_context& ctx ) const
	{
		return std::format_to(
			ctx.out(),
			"mat4:\n\t[{}, {}, {}, {}]\n\t[{}, {}, {}, {}]\n\t[{}, {}, {}, {}]\n\t[{}, {}, {}, {}]",
			mat4[ 0 ][ 0 ],
			mat4[ 0 ][ 1 ],
			mat4[ 0 ][ 2 ],
			mat4[ 0 ][ 3 ],
			mat4[ 1 ][ 0 ],
			mat4[ 1 ][ 1 ],
			mat4[ 1 ][ 2 ],
			mat4[ 1 ][ 3 ],
			mat4[ 2 ][ 0 ],
			mat4[ 2 ][ 1 ],
			mat4[ 2 ][ 2 ],
			mat4[ 2 ][ 3 ],
			mat4[ 3 ][ 0 ],
			mat4[ 3 ][ 1 ],
			mat4[ 3 ][ 2 ],
			mat4[ 3 ][ 3 ] );
	}
};
