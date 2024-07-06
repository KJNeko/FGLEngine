//
// Created by kj16609 on 12/8/23.
//

#pragma once

#include "Descriptor.hpp"
#include "DescriptorPool.hpp"
#include "engine/concepts/is_descriptor.hpp"
#include "engine/concepts/is_descriptor_set.hpp"
#include "engine/concepts/is_empty_descriptor_set.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine::descriptors
{

	template < std::uint16_t set_idx, is_descriptor... Descriptors >
	struct DescriptorSetLayout
	{
		static_assert( sizeof...( Descriptors ) > 0, "Binding set must have at least one binding" );

		static constexpr std::uint16_t m_set_idx { set_idx };

		//! The number of bindings in this set
		static constexpr std::uint16_t descriptor_count { sizeof...( Descriptors ) };

		//! Number of non-empty descriptors
		static constexpr std::uint32_t used_descriptor_count { ( is_layout_descriptor< Descriptors > + ... ) };

		//! The max binding index in this set
		static constexpr std::uint16_t max_binding_idx { getMaxDescriptorIDX< Descriptors... >() };

		static_assert(
			descriptor_count == max_binding_idx + 1,
			"Binding count must equal max binding index. (Use EmptyDescriptor for spaces)" );

		static_assert(
			max_binding_idx + 1 == sizeof...( Descriptors ),
			"Binding count must equal max binding index. No spaces allowed." );

		template < std::uint16_t local_idx >
		using Binding = std::tuple_element_t< local_idx, std::tuple< Descriptors... > >;

		/**
		 * @tparam current_idx Current index to extract
		 * @tparam offset Number to subtract from the current_idx for the array. (Empty bindings are skipped and do not show up in the array)
		 * @return
		 */
		template < std::uint16_t current_idx, std::uint16_t offset >
		static consteval std::array< vk::DescriptorSetLayoutBinding, used_descriptor_count > extractBinding()
		{
			if constexpr ( current_idx == descriptor_count )
			{
				std::array< vk::DescriptorSetLayoutBinding, used_descriptor_count > data {};

				for ( std::uint16_t i = 0; i < used_descriptor_count; ++i )
				{
					data[ i ] = vk::DescriptorSetLayoutBinding(
						std::numeric_limits< std::uint32_t >::max(),
						vk::DescriptorType::eUniformBuffer,
						1,
						vk::ShaderStageFlagBits::eAll,
						nullptr );
				}

				return data;
			}
			else
			{
				using Current = Binding< current_idx >;

				if constexpr ( is_empty_descriptor< Current > )
				{
					return extractBinding< current_idx + 1, offset + 1 >();
				}
				else
				{
					static_assert( is_layout_descriptor< Current >, "Binding is not a layout descriptor" );
					std::array< vk::DescriptorSetLayoutBinding, used_descriptor_count > data {
						extractBinding< current_idx + 1, offset >()
					};
					data[ current_idx - offset ] = Current::m_layout_binding;

					return data;
				}
			}
		}

		template < std::uint16_t current_idx, std::uint16_t offset >
		static consteval std::array< vk::DescriptorBindingFlags, used_descriptor_count > extractBindingFlags()
		{
			if constexpr ( current_idx == descriptor_count )
			{
				std::array< vk::DescriptorBindingFlags, used_descriptor_count > data;

				for ( std::uint16_t i = 0; i < used_descriptor_count; ++i )
				{
					data[ i ] = {};
				}

				return data;
			}
			else
			{
				using Current = Binding< current_idx >;

				if constexpr ( is_empty_descriptor< Current > )
				{
					return extractBindingFlags< current_idx + 1, offset + 1 >();
				}
				else
				{
					static_assert( is_layout_descriptor< Current >, "Binding is not a layout descriptor" );
					std::array< vk::DescriptorBindingFlags, used_descriptor_count > data {
						extractBindingFlags< current_idx + 1, offset >()
					};

					data[ current_idx - offset ] = Current::m_binding_flags;

					return data;
				}
			}
		}

		static consteval std::array< vk::DescriptorSetLayoutBinding, used_descriptor_count > getLayoutBindings()
		{
			return extractBinding< 0, 0 >();
		}

		static consteval std::array< vk::DescriptorBindingFlags, used_descriptor_count > getLayoutBindingFlags()
		{
			return extractBindingFlags< 0, 0 >();
		}

		static vk::raii::DescriptorSetLayout createDescriptorSetLayout()
		{
			static constinit std::array< vk::DescriptorSetLayoutBinding, used_descriptor_count > bindings {
				getLayoutBindings()
			};

			static constinit std::array< vk::DescriptorBindingFlags, used_descriptor_count > flags {
				getLayoutBindingFlags()
			};

			static_assert( bindings.size() == flags.size(), "Binding did not match it's flags" );

			static vk::DescriptorSetLayoutBindingFlagsCreateInfo flags_info {};
			flags_info.pBindingFlags = flags.data();
			flags_info.bindingCount = bindings.size();

			static vk::DescriptorSetLayoutCreateInfo layout_info {};
			layout_info.pNext = VK_NULL_HANDLE;
			layout_info.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
			layout_info.bindingCount = static_cast< std::uint32_t >( bindings.size() );
			layout_info.pBindings = bindings.data();
			layout_info.pNext = &flags_info;

			return Device::getInstance()->createDescriptorSetLayout( layout_info );
		}

	  public:

		DescriptorSetLayout() = delete;

		static vk::raii::DescriptorSetLayout createLayout() { return createDescriptorSetLayout(); }
	};

	template < std::uint16_t set_idx >
	using EmptyDescriptorSet = DescriptorSetLayout< set_idx, EmptyDescriptor< 0 > >;

	namespace internal
	{
		using TestSet = DescriptorSetLayout<
			0,
			ImageDescriptor< 0, vk::ShaderStageFlagBits::eAll >,
			EmptyDescriptor< 1 >,
			AttachmentDescriptor< 2, vk::ShaderStageFlagBits::eAll >,
			UniformDescriptor< 3, vk::ShaderStageFlagBits::eAll >,
			EmptyDescriptor< 4 >,
			UniformDescriptor< 5, vk::ShaderStageFlagBits::eAll | vk::ShaderStageFlagBits::eVertex >,
			UniformDescriptor< 6, vk::ShaderStageFlagBits::eAll > >;

		static_assert( TestSet::descriptor_count == 7 );
	} // namespace internal

	static_assert( is_descriptor_set< EmptyDescriptorSet< 0 > > && is_empty_descriptor_set< EmptyDescriptorSet< 0 > > );
	static_assert( is_descriptor_set< internal::TestSet > && !is_empty_descriptor_set< internal::TestSet > );

} // namespace fgl::engine::descriptors