//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <concepts>
#include <cstdint>

namespace fgl::engine
{
	template < typename T >
	concept is_descriptor_set = requires( std::remove_reference_t< T > t ) {
		{
			t.descriptor_count
		} -> std::same_as< const std::uint16_t& >;
		{
			T::createLayout()
		} -> std::same_as< vk::raii::DescriptorSetLayout >;
	};
} // namespace fgl::engine
