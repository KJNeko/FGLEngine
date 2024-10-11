//
// Created by kj16609 on 12/8/23.
//

#pragma once

#include "Descriptor.hpp"
#include "DescriptorSet.hpp"

namespace fgl::engine
{
	class PipelineBuilder;
}

namespace fgl::engine::descriptors
{
	class DescriptorSet;
	struct Descriptor;

	class DescriptorSetLayout
	{
		std::vector< vk::DescriptorSetLayoutBinding > bindings {};
		std::vector< vk::DescriptorBindingFlags > flags {};

		std::optional< vk::raii::DescriptorSetLayout > m_layout { std::nullopt };

		std::uint16_t m_set_idx;

		DescriptorSetLayout(
			std::uint16_t set_idx, const std::vector< std::reference_wrapper< const Descriptor > >& descriptors );

		DescriptorSetLayout();

	  public:

		//! Used to make creating an empty set VERY EXPLICIT
		inline static DescriptorSetLayout createEmptySet() { return {}; }

		friend class ::fgl::engine::PipelineBuilder;

		template < typename... Args >
		DescriptorSetLayout( const std::uint16_t set_idx, const Args&... descriptors ) :
		  DescriptorSetLayout( set_idx, std::vector< std::reference_wrapper< const Descriptor > > { descriptors... } )
		{}

		std::size_t count() const { return bindings.size(); }

		std::unique_ptr< DescriptorSet > create();
		vk::raii::DescriptorSetLayout createLayout() const;
		vk::raii::DescriptorSetLayout& layout();
	};

} // namespace fgl::engine::descriptors