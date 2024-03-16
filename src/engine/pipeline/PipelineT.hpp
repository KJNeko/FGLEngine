//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include "Pipeline.hpp"
#include "Shader.hpp"
#include "engine/concepts/is_descriptor_set_collection.hpp"
#include "engine/concepts/is_empty_descriptor_set.hpp"
#include "engine/concepts/is_valid_pipeline_input.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/descriptors/createDescriptorSets.hpp"

namespace fgl::engine
{

	template < is_shader_collection ShaderCollection, is_descriptor_set_collection DescriptorSetCollection >
	class PipelineT : public internal::Pipeline
	{
		//If the first descriptor set is a constant range, then the pipeline has a constant range
		constexpr static bool has_constant_range { DescriptorSetCollection::has_constant_range };
		constexpr static std::uint16_t binding_sets { DescriptorSetCollection::binding_sets };
		constexpr static bool has_binding_sets { binding_sets != 0 };

		//! Returns the binding type assocaited with the index
		template < std::uint16_t binding_set_idx >
		using BindingSet = DescriptorSetCollection::template BindingSet< binding_set_idx >;

		constexpr static std::uint16_t max_binding_set { DescriptorSetCollection::max_binding_set };

		constexpr static std::uint16_t set_count { DescriptorSetCollection::set_count };

		constexpr static std::uint16_t empty_sets { DescriptorSetCollection::empty_sets };

		static_assert(
			( set_count == 0 && has_constant_range ) || ( set_count == ( max_binding_set + 1 ) ),
			"Binding sets must not have any spaces (Use EmptySet<idx>)" );

		template < std::uint16_t idx >
		consteval static std::uint16_t emptyBindingsAfterIDX()
		{
			if constexpr ( idx == max_binding_set )
				return 0;
			else
			{
				return ( is_empty_descriptor_set< BindingSet< idx > > ? 1 : 0 ) + emptyBindingsAfterIDX< idx + 1 >();
			}
		}

		const vk::PushConstantRange* getRange()
		{
			if constexpr ( has_constant_range )
			{
				return &DescriptorSetCollection::template DescriptorSet< 0 >::m_range;
			}
			else
			{
				return VK_NULL_HANDLE;
			}
		}

		template < std::uint16_t start_idx >
		consteval static std::uint16_t getConcurrentSetCount()
		{
			if constexpr ( start_idx > max_binding_set )
				return 0;
			else
			{
				if constexpr ( !is_empty_descriptor_set< BindingSet< start_idx > > )
					return 1 + getConcurrentSetCount< start_idx + 1 >();
				else
					return 0;
			}
		}

		template < std::uint16_t start_idx >
		static consteval std::uint16_t emptySetsBeforeIDX()
		{
			if constexpr ( start_idx == 0 )
				return 0;
			else
				return emptySetsBeforeIDX< start_idx - 1 >() + is_empty_descriptor_set< BindingSet< start_idx > >;
		}

		vk::PipelineLayout createLayout( Device& device )
		{
			if ( m_layout != VK_NULL_HANDLE ) return m_layout;

			typename DescriptorSetCollection::LayoutArray layouts { DescriptorSetCollection::createDescriptorSets() };

			vk::PipelineLayoutCreateInfo pipeline_layout_info {};
			pipeline_layout_info.setLayoutCount = has_binding_sets ? static_cast< uint32_t >( layouts.size() ) : 0;
			pipeline_layout_info.pSetLayouts = has_binding_sets ? layouts.data() : VK_NULL_HANDLE;
			pipeline_layout_info.pushConstantRangeCount = has_constant_range ? 1 : 0;
			pipeline_layout_info.pPushConstantRanges = has_constant_range ? getRange() : VK_NULL_HANDLE;

			if ( device.device().createPipelineLayout( &pipeline_layout_info, nullptr, &m_layout )
			     != vk::Result::eSuccess )
				throw std::runtime_error( "Failed to create pipeline layout" );

			return m_layout;
		}

		PipelineConfigInfo& populate( PipelineConfigInfo& info, Device& device )
		{
			info.layout = createLayout( device );

			return info;
		}

	  public:

		vk::PipelineLayout getLayout() { return m_layout; }

		void bindDescriptor( vk::CommandBuffer cmd_buffer, std::uint16_t set_idx, DescriptorSet& descriptor )
		{
			cmd_buffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_layout, set_idx, 1, &( descriptor.getSet() ), 0, nullptr );
		}

		template < typename TPushData >
		void pushConstant( vk::CommandBuffer cmd_buffer, TPushData& data )
		{
			if constexpr ( has_constant_range )
			{
				using PushConstantType = typename DescriptorSetCollection::PushConstantT;
				static_assert(
					std::same_as< TPushData, typename PushConstantType::DataType >,
					"Push constant data type mismatch" );

				PushConstantType::push( cmd_buffer, m_layout, data );
			}
			else
				assert( "Attempted to push constant to pipeline without push constant range" );
		}

		PipelineT( Device& device, PipelineConfigInfo& info ) : Pipeline( device )
		{
			populate( info, device );

			auto shaders { ShaderCollection::loadShaders() };

			createGraphicsPipeline( shaders, info );
		}
	};

} // namespace fgl::engine