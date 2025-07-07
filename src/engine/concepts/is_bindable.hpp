//
// Created by kj16609 on 12/31/23.
//

#pragma once

#include <concepts>

namespace fgl::engine
{
	template < typename T >
	concept is_bindable_buffer = requires( T t ) {
		{
			t.descriptorInfo()
		} -> std::same_as< vk::DescriptorBufferInfo >;
	};

	template < typename T >
	concept is_bindable_image = requires( T t ) {
		{
			t.descriptorInfo()
		} -> std::same_as< vk::DescriptorImageInfo >;
	};

	template < typename T > concept is_bindable = is_bindable_image< T > || is_bindable_buffer< T >;

} // namespace fgl::engine
