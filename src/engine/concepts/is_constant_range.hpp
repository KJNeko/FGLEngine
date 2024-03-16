//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace fgl::engine
{
	template < typename T >
	concept is_constant_range = requires( T t ) {
		{
			t.m_range
		} -> std::same_as< const vk::PushConstantRange& >;
	};
} // namespace fgl::engine