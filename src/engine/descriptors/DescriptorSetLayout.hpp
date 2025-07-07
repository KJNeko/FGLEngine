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

		DescriptorSetLayout( DescriptorIDX set_idx, const std::vector< const Descriptor* >& descriptors );

		DescriptorSetLayout();

	  public:

		//! Used to make creating an empty set VERY EXPLICIT
		static DescriptorSetLayout createEmptySet() { return {}; }

		friend class ::fgl::engine::PipelineBuilder;

// -fanalyzer thinks we leak memory here. We don't. Disable -Wanalyzer-malloc-leak
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"

		template < typename... Args >
		static DescriptorSetLayout create( const DescriptorIDX set_idx, const Args&... descriptors )
		{
			const std::vector< const Descriptor* > descriptors_ref { ( &descriptors )... };
			return { set_idx, descriptors_ref };
		}

#pragma GCC diagnostic pop

		[[nodiscard]] std::size_t count() const { return m_bindings.size(); }

		[[nodiscard]] DescriptorSetPtr create();

		[[nodiscard]] vk::raii::DescriptorSetLayout createLayout() const;
		vk::raii::DescriptorSetLayout& layout();
	};

} // namespace fgl::engine::descriptors