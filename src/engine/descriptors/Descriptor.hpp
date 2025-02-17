//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include <cstdint>

#include "engine/memory/buffers/Buffer.hpp"

namespace fgl::engine::descriptors
{

	struct Descriptor
	{
		std::uint16_t m_index;
		vk::DescriptorType m_type;
		vk::ShaderStageFlags m_stage_flags;
		std::uint16_t m_count;
		vk::DescriptorBindingFlags m_binding_flags;

		consteval Descriptor() = delete;

		constexpr Descriptor(
			const std::uint16_t binding_idx,
			const vk::DescriptorType type,
			const vk::ShaderStageFlags stage_flags,
			const std::uint16_t count = 1,
			const vk::DescriptorBindingFlags binding_flags = static_cast< vk::DescriptorBindingFlags >( 0 ) ) :
		  m_index( binding_idx ),
		  m_type( type ),
		  m_stage_flags( stage_flags ),
		  m_count( count ),
		  m_binding_flags( binding_flags ),
		  m_layout_binding( generateLayoutBinding() )
		{}

		constexpr vk::DescriptorSetLayoutBinding generateLayoutBinding()
		{
			vk::DescriptorSetLayoutBinding layout_binding;
			layout_binding.binding = m_index;
			layout_binding.descriptorType = m_type;
			layout_binding.descriptorCount = m_count;
			layout_binding.stageFlags = m_stage_flags;
			layout_binding.pImmutableSamplers = VK_NULL_HANDLE;
			return layout_binding;
		}

		vk::DescriptorSetLayoutBinding m_layout_binding;
	};

	struct ImageDescriptor : Descriptor
	{
		ImageDescriptor() = delete;

		constexpr ImageDescriptor( std::uint16_t idx, vk::ShaderStageFlags stage_flags ) :
		  Descriptor( idx, vk::DescriptorType::eSampledImage, stage_flags )
		{}
	};

	struct AttachmentDescriptor : Descriptor
	{
		AttachmentDescriptor() = delete;

		constexpr AttachmentDescriptor( std::uint16_t idx, vk::ShaderStageFlags stage_flags ) :
		  Descriptor( idx, vk::DescriptorType::eInputAttachment, stage_flags )
		{}
	};

	struct StorageDescriptor : Descriptor
	{
		StorageDescriptor() = delete;

		StorageDescriptor( std::uint16_t idx, vk::ShaderStageFlags stage_flags ) :
		  Descriptor( idx, vk::DescriptorType::eStorageBuffer, stage_flags )
		{}
	};

	struct UniformDescriptor : Descriptor
	{
		UniformDescriptor() = delete;

		UniformDescriptor( const std::uint16_t idx, vk::ShaderStageFlags stage_flags ) :
		  Descriptor( idx, vk::DescriptorType::eUniformBuffer, stage_flags )
		{}
	};

} // namespace fgl::engine::descriptors
