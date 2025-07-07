//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>

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

	  private:

		[[nodiscard]] constexpr vk::DescriptorSetLayoutBinding generateLayoutBinding() const
		{
			vk::DescriptorSetLayoutBinding layout_binding;
			layout_binding.binding = m_index;
			layout_binding.descriptorType = m_type;
			layout_binding.descriptorCount = m_count;
			layout_binding.stageFlags = m_stage_flags;
			layout_binding.pImmutableSamplers = nullptr;
			return layout_binding;
		}

	  public:

		constexpr Descriptor(
			const std::uint16_t binding_idx,
			const vk::DescriptorType type,
			const vk::ShaderStageFlags stage_flags,
			const std::uint16_t count = 1,
			const vk::DescriptorBindingFlags binding_flags = vk::DescriptorBindingFlags {} ) :
		  m_index( binding_idx ),
		  m_type( type ),
		  m_stage_flags( stage_flags ),
		  m_count( count ),
		  m_binding_flags( binding_flags ),
		  m_layout_binding( generateLayoutBinding() )
		{}

		vk::DescriptorSetLayoutBinding m_layout_binding;

		virtual ~Descriptor() = default;
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

		constexpr StorageDescriptor( std::uint16_t idx, vk::ShaderStageFlags stage_flags ) :
		  Descriptor( idx, vk::DescriptorType::eStorageBuffer, stage_flags )
		{}
	};

	struct UniformDescriptor : Descriptor
	{
		UniformDescriptor() = delete;

		constexpr UniformDescriptor( const std::uint16_t idx, vk::ShaderStageFlags stage_flags ) :
		  Descriptor( idx, vk::DescriptorType::eUniformBuffer, stage_flags )
		{}
	};

} // namespace fgl::engine::descriptors
