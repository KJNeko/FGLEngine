//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include "../../concepts/is_descriptor_set_collection.hpp"
#include "../../concepts/is_empty_descriptor_set.hpp"
#include "../../descriptors/DescriptorSet.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"

namespace fgl::engine
{

	template < is_shader_collection ShaderCollection, is_descriptor_set_collection DescriptorSetCollection >
	class PipelineT : public internal::Pipeline
	{
		//If the first descriptor set is a constant range, then the pipeline has a constant range
		constexpr static bool has_constant_range { DescriptorSetCollection::has_constant_range };
		constexpr static std::uint16_t binding_sets { DescriptorSetCollection::binding_sets };
		constexpr static bool has_binding_sets { binding_sets != 0 };

		constexpr static std::uint16_t max_binding_set { DescriptorSetCollection::max_binding_set };

		constexpr static std::uint16_t set_count { DescriptorSetCollection::set_count };

		constexpr static std::uint16_t empty_sets { DescriptorSetCollection::empty_sets };

		//! Returns the binding type assocaited with the index
		template < std::uint16_t binding_set_idx >
		using BindingSet = typename DescriptorSetCollection::template BindingSet< binding_set_idx >;

		static_assert(
			set_count == 0 || ( set_count == ( max_binding_set + 1 ) ),
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

		vk::raii::PipelineLayout createLayout( [[maybe_unused]] Device& device )
		{
			const auto layouts { DescriptorSetCollection::createDescriptorSets() };
			std::vector< vk::DescriptorSetLayout > vk_layouts {};
			vk_layouts.reserve( layouts.size() );

			for ( const vk::raii::DescriptorSetLayout& layout : layouts )
			{
				vk_layouts.emplace_back( *layout );
			}

			vk::PipelineLayoutCreateInfo pipeline_layout_info {};
			pipeline_layout_info.setLayoutCount = has_binding_sets ? static_cast< uint32_t >( vk_layouts.size() ) : 0;
			pipeline_layout_info.pSetLayouts = has_binding_sets ? vk_layouts.data() : VK_NULL_HANDLE;
			pipeline_layout_info.pushConstantRangeCount = has_constant_range ? 1 : 0;
			pipeline_layout_info.pPushConstantRanges = has_constant_range ? getRange() : VK_NULL_HANDLE;

			return Device::getInstance()->createPipelineLayout( pipeline_layout_info );
		}

	  public:

		void bindDescriptor(
			vk::raii::CommandBuffer& cmd_buffer, std::uint16_t set_idx, descriptors::DescriptorSet& descriptor )
		{
			const std::vector< vk::DescriptorSet > sets { *descriptor };
			const std::vector< std::uint32_t > offsets {};

			cmd_buffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, m_layout, set_idx, sets, offsets );
		}

		template < typename TPushData >
		void pushConstant( vk::raii::CommandBuffer& cmd_buffer, TPushData& data )
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

		PipelineT( Device& device, PipelineConfigInfo&& info ) :
		  Pipeline(
			  device,
			  createLayout( device ),
			  std::forward< PipelineConfigInfo >( info ),
			  ShaderCollection::loadShaders() )
		{}
	};

} // namespace fgl::engine