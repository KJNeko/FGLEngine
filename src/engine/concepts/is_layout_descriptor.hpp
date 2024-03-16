//
// Created by kj16609 on 3/13/24.
//

#pragma once

namespace fgl::engine
{
	template < typename T >
	concept is_layout_descriptor = requires( T t ) {
		{
			t.m_binding_idx
		} -> std::same_as< const std::uint16_t& >;
		{
			t.m_layout_binding
		} -> std::same_as< const vk::DescriptorSetLayoutBinding& >;
		{
			t.m_descriptor_type
		} -> std::same_as< const vk::DescriptorType& >;
	};
}