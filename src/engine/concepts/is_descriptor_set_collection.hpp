//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <concepts>
#include <cstdint>

namespace fgl::engine
{

	template < typename T >
	concept is_descriptor_set_collection = requires( T t ) {
		typename T::DescriptorSetTuple;
		{
			t.DescriptorSetCount
		} -> std::same_as< const std::uint64_t& >;
	};

} // namespace fgl::engine