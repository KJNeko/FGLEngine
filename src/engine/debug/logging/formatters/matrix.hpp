//
// Created by kj16609 on 10/1/24.
//

#pragma once
#include <glm/fwd.hpp>

#include <format>

template <>
struct std::formatter< glm::vec4 >
{
	constexpr format_parse_context::iterator parse( format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const glm::vec4& vec, format_context& ctx ) const;
};

template <>
struct std::formatter< glm::vec3 >
{
	constexpr format_parse_context::iterator parse( format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const glm::vec3& vec, format_context& ctx ) const;
};

template <>
struct std::formatter< glm::mat4 >
{
	constexpr format_parse_context::iterator parse( format_parse_context& ctx ) { return ctx.begin(); }

	format_context::iterator format( const glm::mat4& mat, format_context& ctx ) const;
};