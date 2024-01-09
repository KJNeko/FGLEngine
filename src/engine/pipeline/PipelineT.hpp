//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include "Pipeline.hpp"
#include "engine/PushConstant.hpp"
#include "engine/descriptors/Descriptor.hpp"
#include "engine/descriptors/DescriptorSetLayout.hpp"

namespace fgl::engine
{

	template < typename T > concept is_valid_pipeline_input = is_constant_range< T > || is_descriptor_set< T >;

	template <
		std::uint16_t size,
		std::uint16_t current_idx,
		is_valid_pipeline_input CurrentSet,
		is_valid_pipeline_input... Sets >
	void createDescriptorSets( std::array< vk::DescriptorSetLayout, size >& out, Device& device )
	{
		if constexpr ( size == 0 )
			return;
		else
		{
			static_assert( size > 0, "Size must be greater than 0" );
			static_assert( current_idx < size, "Current index must be less than size" );

			if constexpr ( is_descriptor_set< CurrentSet > )
			{
				out[ current_idx ] = CurrentSet::createDescriptorSetLayout( device );
				assert( out[ current_idx ] != VK_NULL_HANDLE && "createDescriptorSetLayout returned VK_NULL_HANDLE" );
				std::cout << "Created descriptor set layout for binding set " << current_idx << std::endl;
				if constexpr ( sizeof...( Sets ) > 0 )
					createDescriptorSets< size, current_idx + 1, Sets... >( out, device );
			}
			else if constexpr ( is_constant_range< CurrentSet > )
			{
				if constexpr ( sizeof...( Sets ) > 0 ) // We don't want to increase the size
					createDescriptorSets< size, current_idx, Sets... >( out, device );
				else
					return;
			}
			else
			{
				static_assert( false, "Invalid input" );
			}
		}
	}

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

	template < typename... DescriptorSets >
		requires( (is_valid_pipeline_input< DescriptorSets >) && ... )
	class PipelineT : public Pipeline
	{
		//If the first descriptor set is a constant range, then the pipeline has a constant range
		constexpr static bool has_constant_range {
			is_constant_range< std::tuple_element_t< 0, std::tuple< DescriptorSets... > > >
		};
		constexpr static std::uint16_t binding_sets { ( is_descriptor_set< DescriptorSets > + ... ) };
		constexpr static bool has_binding_sets { binding_sets != 0 };

		//! Returns the binding type assocaited with the index
		template < std::uint16_t binding_set_idx >
		using BindingSet =
			std::tuple_element_t< binding_set_idx + ( has_constant_range ? 1 : 0 ), std::tuple< DescriptorSets... > >;

		constexpr static std::uint16_t max_binding_set { getMaxBindingSetIDX< DescriptorSets... >() };

		constexpr static std::uint16_t set_count { ( is_descriptor_set< DescriptorSets > + ... ) };

		constexpr static std::uint16_t empty_sets { ( is_empty_descriptor_set< DescriptorSets > + ... ) };

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
				return &std::tuple_element_t< 0, std::tuple< DescriptorSets... > >::m_range;
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

			std::array< vk::DescriptorSetLayout, sizeof...( DescriptorSets ) - has_constant_range > layouts;

			createDescriptorSets< layouts.size(), 0, DescriptorSets... >( layouts, device );

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
				using PushConstantType = typename std::tuple_element_t< 0, std::tuple< DescriptorSets... > >;
				static_assert(
					std::same_as< TPushData, typename PushConstantType::DataType >,
					"Push constant data type mismatch" );

				PushConstantType::push( cmd_buffer, m_layout, data );
			}
			else
				assert( "Attempted to push constant to pipeline without push constant range" );
		}

		PipelineT(
			Device& device,
			PipelineConfigInfo& info,
			std::filesystem::path vertex_shader,
			std::filesystem::path frag_shader ) :
		  Pipeline( device )
		{
			populate( info, device );
			createGraphicsPipeline( vertex_shader, frag_shader, info );
		}
	};

} // namespace fgl::engine