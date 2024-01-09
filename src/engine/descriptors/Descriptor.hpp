//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include <cstdint>

#include "engine/buffers/Buffer.hpp"
#include "engine/concepts/is_bindable.hpp"
#include "engine/concepts/is_descriptor.hpp"

namespace fgl::engine
{

	template <
		std::uint16_t binding_idx,
		vk::DescriptorType descriptor_type,
		vk::ShaderStageFlags stage_flags = vk::ShaderStageFlagBits::eAll,
		std::uint16_t binding_count = 1 >
	struct Descriptor
	{
		static constexpr std::uint16_t m_binding_idx { binding_idx };
		static constexpr std::uint16_t m_count { binding_count };

		static constexpr vk::DescriptorType m_descriptor_type { descriptor_type };

		consteval static vk::DescriptorSetLayoutBinding generateLayoutBinding()
		{
			vk::DescriptorSetLayoutBinding layout_binding {};
			layout_binding.binding = binding_idx;
			layout_binding.descriptorType = descriptor_type;
			layout_binding.descriptorCount = binding_count;
			layout_binding.stageFlags = stage_flags;
			layout_binding.pImmutableSamplers = VK_NULL_HANDLE;
			return layout_binding;
		}

		/**
		 * The layout binding is used during pipeline creation to provide information to the pipeline about the descriptor set layout.
		 * This is used to construct the actual layout and can be done at compiletime.
		 */
		static constexpr vk::DescriptorSetLayoutBinding m_layout_binding { generateLayoutBinding() };
	};

	template < std::uint16_t binding_idx, vk::ShaderStageFlags stage_flags >
	using ImageDescriptor = Descriptor< binding_idx, vk::DescriptorType::eSampledImage, stage_flags >;

	template < std::uint16_t binding_idx, vk::ShaderStageFlags stage_flags >
	using AttachmentDescriptor = Descriptor< binding_idx, vk::DescriptorType::eInputAttachment, stage_flags >;

	template < std::uint16_t binding_idx, vk::ShaderStageFlags stage_flags >
	using StorageDescriptor = Descriptor< binding_idx, vk::DescriptorType::eStorageBuffer, stage_flags >;

	template < std::uint16_t binding_idx, vk::ShaderStageFlags stage_flags >
	using UniformDescriptor = Descriptor< binding_idx, vk::DescriptorType::eUniformBuffer, stage_flags >;

	template < std::uint16_t idx >
	struct EmptyDescriptor
	{
		static constexpr std::uint16_t m_binding_idx { idx };
		static constexpr bool is_empty { true };
	};

	static_assert( is_empty_descriptor< EmptyDescriptor< 0 > > );

	//! Returns the maximum binding index for a list of given descriptors
	template < is_descriptor Current, is_descriptor... Bindings >
	consteval std::uint16_t getMaxDescriptorIDX()
	{
		if constexpr ( sizeof...( Bindings ) == 0 )
		{
			return Current::m_binding_idx;
		}
		else
		{
			return std::max( Current::m_binding_idx, getMaxDescriptorIDX< Bindings... >() );
		}
	}

} // namespace fgl::engine
