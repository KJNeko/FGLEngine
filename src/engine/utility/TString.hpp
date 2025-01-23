//
// Created by kj16609 on 1/22/25.
//
#pragma once

#include <memory>

namespace fgl::engine::debug
{

	template < std::size_t N >
	struct TString
	{
		char m_data[ N ] {};

		[[nodiscard]] constexpr std::size_t size() const { return N; }

		[[nodiscard]] constexpr const char* c_str() const { return m_data; }

		operator std::string_view() const { return std::string_view { m_data, N - 1 }; }

		TString() = delete;
		TString( const TString& ) = delete;
		TString& operator=( const TString& ) = delete;
		TString( TString&& ) = delete;
		TString& operator=( TString&& ) = delete;

		consteval TString( const char ( &ptr )[ N ] ) noexcept
		{
			//static_assert( ptr[ N - 1 ] == '\0', "String does not end in null terminator" );
			std::ranges::copy( ptr, m_data );
		}
	};

} // namespace fgl::engine::debug