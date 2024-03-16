//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include "engine/concepts/is_empty_descriptor_set.hpp"
#include "engine/concepts/is_valid_pipeline_input.hpp"
#include "engine/descriptors/createDescriptorSets.hpp"

namespace fgl::engine
{

	template < is_descriptor_set Current, is_valid_pipeline_input... Sets >
	consteval std::uint16_t getMaxBindingSetIDX()
	{
		if constexpr ( sizeof...( Sets ) == 0 )
			return Current::m_set_idx;
		else
		{
			constexpr auto current_idx { Current::m_set_idx };
			constexpr auto next_idx { getMaxBindingSetIDX< Sets... >() };
			return std::max( current_idx, next_idx );
		}
	}

	template < is_constant_range Current, is_valid_pipeline_input... Sets >
	consteval std::uint16_t getMaxBindingSetIDX()
	{
		if constexpr ( sizeof...( Sets ) == 0 )
			return 0;
		else
			return getMaxBindingSetIDX< Sets... >();
	}

	template < is_valid_pipeline_input... DescriptorSets >
	struct DescriptorSetCollection
	{
		using DescriptorSetTuple = std::tuple< DescriptorSets... >;

		constexpr static std::uint64_t DescriptorSetCount { sizeof...( DescriptorSets ) };

		//If the first descriptor set is a constant range, then the pipeline has a constant range
		constexpr static bool has_constant_range {
			is_constant_range< std::tuple_element_t< 0, std::tuple< DescriptorSets... > > >
		};

		constexpr static std::uint16_t binding_sets { ( is_descriptor_set< DescriptorSets > + ... ) };

		constexpr static std::uint16_t max_binding_set { getMaxBindingSetIDX< DescriptorSets... >() };

		constexpr static std::uint16_t set_count { ( is_descriptor_set< DescriptorSets > + ... ) };

		constexpr static std::uint16_t empty_sets { ( is_empty_descriptor_set< DescriptorSets > + ... ) };

		using LayoutArray = std::array< vk::DescriptorSetLayout, DescriptorSetCount - has_constant_range >;

		static LayoutArray createDescriptorSets()
		{
			LayoutArray layouts;
			createDescriptorSetsT< layouts.size(), 0, DescriptorSets... >( layouts );
			return layouts;
		}

		template < std::uint64_t IDX >
			requires( IDX < DescriptorSetCount )
		using DescriptorSet = std::tuple_element_t< IDX, DescriptorSetTuple >;

		template < std::uint64_t BindingIDX >
		using BindingSet = DescriptorSet< BindingIDX + ( has_constant_range ? 1 : 0 ) >;

		using PushConstantT = BindingSet< 0 >;
	};

} // namespace fgl::engine