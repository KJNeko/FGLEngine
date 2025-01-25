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
		std::vector< vk::DescriptorSetLayoutBinding > m_bindings {};
		std::vector< vk::DescriptorBindingFlags > m_flags {};

		std::optional< vk::raii::DescriptorSetLayout > m_layout { std::nullopt };

		DescriptorIDX m_set_idx;

		std::size_t m_binding_count;

		DescriptorSetLayout(
			DescriptorIDX set_idx, const std::vector< std::reference_wrapper< const Descriptor > >& descriptors );

		DescriptorSetLayout();

	  public:

		//! Used to make creating an empty set VERY EXPLICIT
		inline static DescriptorSetLayout createEmptySet() { return {}; }

		friend class ::fgl::engine::PipelineBuilder;

		template < typename... Args >
		DescriptorSetLayout( const DescriptorIDX set_idx, const Args&... descriptors ) :
		  DescriptorSetLayout( set_idx, std::vector< std::reference_wrapper< const Descriptor > > { descriptors... } )
		{}

		[[nodiscard]] std::size_t count() const { return m_bindings.size(); }

		std::unique_ptr< DescriptorSet > create();
		[[nodiscard]] vk::raii::DescriptorSetLayout createLayout() const;
		vk::raii::DescriptorSetLayout& layout();
	};

} // namespace fgl::engine::descriptors