//
// Created by kj16609 on 10/3/24.
//

#pragma once

#include <glm/fwd.hpp>

#include <format>

template <>
struct std::formatter< glm::quat >
{
	constexpr format_parse_context::iterator parse( format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const glm::quat& quat, format_context& ctx ) const;
};