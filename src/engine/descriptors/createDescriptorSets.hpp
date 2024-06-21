//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include "engine/concepts/is_valid_pipeline_input.hpp"

namespace fgl::engine
{

	template < is_valid_pipeline_input CurrentSet, is_valid_pipeline_input... Sets >
	std::vector< vk::raii::DescriptorSetLayout > createDescriptorSetsT( std::vector< vk::raii::DescriptorSetLayout >
	                                                                        out )
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
			return createDescriptorSetsT< Sets... >( std::move( out ) );
		}

		return out;
	}

	template < is_valid_pipeline_input CurrentSet, is_valid_pipeline_input... Sets >
	std::vector< vk::raii::DescriptorSetLayout > createDescriptorSetsT()
	{
		std::vector< vk::raii::DescriptorSetLayout > out {};

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
			return createDescriptorSetsT< Sets... >( std::move( out ) );
		}

		return out;
	}

} // namespace fgl::engine