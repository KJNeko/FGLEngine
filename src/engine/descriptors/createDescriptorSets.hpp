//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include "engine/concepts/is_valid_pipeline_input.hpp"

namespace fgl::engine::descriptors
{

	template < is_valid_pipeline_input CurrentSet, is_valid_pipeline_input... Sets >
	FGL_FORCE_INLINE inline void createDescriptorSetsT( std::vector< vk::raii::DescriptorSetLayout >& out )
	{
		if constexpr ( is_descriptor_set< CurrentSet > )
		{
			vk::raii::DescriptorSetLayout layout { CurrentSet::createDescriptorSetLayout() };
			out.emplace_back( std::move( layout ) );
		}
		else if constexpr ( is_constant_range< CurrentSet > )
		{
			//noop
		}
		else
		{
			static_assert( false, "Invalid input" );
		}

		if constexpr ( sizeof...( Sets ) > 0 )
		{
			return createDescriptorSetsT< Sets... >( out );
		}
	}

} // namespace fgl::engine::descriptors