//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <cstdint>
#include <iostream>

#include "engine/concepts/is_valid_pipeline_input.hpp"

namespace fgl::engine
{

	template <
		std::uint16_t size,
		std::uint16_t current_idx,
		is_valid_pipeline_input CurrentSet,
		is_valid_pipeline_input... Sets >
	void createDescriptorSetsT( std::array< vk::DescriptorSetLayout, size >& out )
	{
		if constexpr ( size == 0 )
			return;
		else
		{
			static_assert( size > 0, "Size must be greater than 0" );
			static_assert( current_idx < size, "Current index must be less than size" );

			if constexpr ( is_descriptor_set< CurrentSet > )
			{
				out[ current_idx ] = CurrentSet::createDescriptorSetLayout();
				assert( out[ current_idx ] != VK_NULL_HANDLE && "createDescriptorSetLayout returned VK_NULL_HANDLE" );
				std::cout << "Created descriptor set layout for binding set " << current_idx << std::endl;
				if constexpr ( sizeof...( Sets ) > 0 ) createDescriptorSetsT< size, current_idx + 1, Sets... >( out );
			}
			else if constexpr ( is_constant_range< CurrentSet > )
			{
				if constexpr ( sizeof...( Sets ) > 0 ) // We don't want to increase the size
					createDescriptorSetsT< size, current_idx, Sets... >( out );
				else
					return;
			}
			else
			{
				static_assert( false, "Invalid input" );
			}
		}
	}
} // namespace fgl::engine