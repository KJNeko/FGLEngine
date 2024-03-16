//
// Created by kj16609 on 3/13/24.
//

#pragma once

namespace fgl::engine
{
	/**
	 * Descriptor must have a `static constexpr bool is_empty` member with the value `true`
	 * Descriptor must also be valid in `is_descriptor`
	 * @tparam T
	 */
	template < typename T >
	concept is_empty_descriptor = requires( T t ) {
		{
			t.m_binding_idx
		} -> std::same_as< const std::uint16_t& >;
		{
			t.is_empty
		} -> std::same_as< const bool& >;
	} && T::is_empty;

} // namespace fgl::engine