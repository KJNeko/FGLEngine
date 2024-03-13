//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include "engine/concepts/is_descriptor.hpp"

namespace fgl::engine
{

	template < typename T >
	concept is_descriptor_set = requires( std::remove_reference_t< T > t ) {
		{
			t.descriptor_count
		} -> std::same_as< const std::uint16_t& >;
		{
			T::createLayout()
		} -> std::same_as< vk::DescriptorSetLayout >;
	};

	template < typename T >
	concept is_empty_descriptor_set = is_descriptor_set< T > && ( T::descriptor_count == 1 )
	                               && is_empty_descriptor< typename T::template Binding< 0 > >;

	template < typename T >
	concept is_constant_range = requires( T t ) {
		{
			t.m_range
		} -> std::same_as< const vk::PushConstantRange& >;
	};

	template < typename T > concept is_valid_pipeline_input = is_constant_range< T > || is_descriptor_set< T >;
} // namespace fgl::engine
