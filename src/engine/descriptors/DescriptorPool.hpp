//
// Created by kj16609 on 12/14/23.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <unordered_map>

namespace fgl::engine
{
	class Device;
}

namespace fgl::engine::descriptors
{

	static const std::unordered_map< vk::DescriptorType, float > descriptor_allocation_ratios {
		{ vk::DescriptorType::eUniformBuffer, 2.0f }, { vk::DescriptorType::eCombinedImageSampler, 2.0f }
	};

	class DescriptorPool
	{
		vk::raii::DescriptorPool m_pool;

		DescriptorPool( Device& device, std::uint32_t set_count );

	  public:

		DescriptorPool( const DescriptorPool& other ) = delete;
		DescriptorPool( DescriptorPool&& other ) = delete;
		DescriptorPool& operator=( const DescriptorPool& other ) = delete;
		DescriptorPool& operator=( DescriptorPool&& other ) = delete;

		vk::raii::DescriptorPool& getPool() { return m_pool; }

		VkDescriptorPool operator*() { return *m_pool; }

		static DescriptorPool& init( Device& device );
		[[nodiscard]] static DescriptorPool& getInstance();

		[[nodiscard]] vk::raii::DescriptorSet allocateSet( vk::raii::DescriptorSetLayout& layout );
	};
} // namespace fgl::engine::descriptors

namespace fgl::engine
{
	using descriptors::DescriptorPool;
}