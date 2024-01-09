//
// Created by kj16609 on 12/14/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <unordered_map>

namespace fgl::engine
{
	class Device;

	static const std::unordered_map< vk::DescriptorType, float > descriptor_allocation_ratios {
		{ vk::DescriptorType::eUniformBuffer, 2.0f }, { vk::DescriptorType::eCombinedImageSampler, 2.0f }
	};

	class DescriptorPool
	{
		vk::DescriptorPool m_pool {};

		DescriptorPool( Device& device, std::uint32_t set_count );

	  public:

		DescriptorPool( const DescriptorPool& other ) = delete;
		DescriptorPool( DescriptorPool&& other ) = delete;
		DescriptorPool& operator=( const DescriptorPool& other ) = delete;
		DescriptorPool& operator=( DescriptorPool&& other ) = delete;

		vk::DescriptorPool getPool() const
		{
			assert( m_pool && "DescriptorPool::getVkPool() called on null pool" );

			return m_pool;
		}

		VkDescriptorPool getVkPool() const
		{
			assert( m_pool && "DescriptorPool::getVkPool() called on null pool" );

			return m_pool;
		}

		static DescriptorPool& init( Device& device );
		[[nodiscard]] static DescriptorPool& getInstance();

		[[nodiscard]] vk::DescriptorSet allocateSet( vk::DescriptorSetLayout& layout );
		void deallocSet( vk::DescriptorSet& set );
	};
} // namespace fgl::engine