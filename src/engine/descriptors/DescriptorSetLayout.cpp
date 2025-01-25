//
// Created by kj16609 on 10/9/24.
//

#include "DescriptorSetLayout.hpp"

#include "DescriptorSet.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::descriptors
{

	DescriptorSetLayout::DescriptorSetLayout(
		const DescriptorIDX set_idx, const std::vector< std::reference_wrapper< const Descriptor > >& descriptors ) :
	  m_set_idx( set_idx ),
	  m_binding_count( descriptors.size() )
	{
		FGL_ASSERT( descriptors.size() > 0, "Must have more then 1 descriptor set" );

		for ( const auto& descriptor_w : descriptors )
		{
			const auto& descriptor { descriptor_w.get() };
			vk::DescriptorSetLayoutBinding binding {};
			binding.binding = descriptor.m_index;
			binding.descriptorType = descriptor.m_type;
			binding.descriptorCount = descriptor.m_count;
			binding.stageFlags = descriptor.m_stage_flags;
			binding.pImmutableSamplers = VK_NULL_HANDLE;

			m_bindings.emplace_back( binding );

			m_flags.emplace_back( descriptor.m_binding_flags );
		}
	}

	DescriptorSetLayout::DescriptorSetLayout() :
	  m_set_idx( std::numeric_limits< std::uint16_t >::max() ),
	  m_binding_count( std::numeric_limits< std::size_t >::min() )
	{}

	std::unique_ptr< DescriptorSet > DescriptorSetLayout::create()
	{
		if ( !m_layout.has_value() ) m_layout = createLayout();
		return std::make_unique< DescriptorSet >( *m_layout, m_set_idx, m_binding_count );
	}

	vk::raii::DescriptorSetLayout DescriptorSetLayout::createLayout() const
	{
		vk::DescriptorSetLayoutBindingFlagsCreateInfo flags_info {};
		flags_info.setBindingFlags( m_flags );

		vk::DescriptorSetLayoutCreateInfo info {};
		info.setFlags( vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool );
		info.setBindings( m_bindings );
		info.setPNext( &flags_info );

		return Device::getInstance()->createDescriptorSetLayout( info );
	}

	vk::raii::DescriptorSetLayout& DescriptorSetLayout::layout()
	{
		if ( !m_layout.has_value() ) m_layout = createLayout();

		return *m_layout;
	}

} // namespace fgl::engine::descriptors
