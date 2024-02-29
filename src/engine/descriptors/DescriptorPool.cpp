//
// Created by kj16609 on 12/14/23.
//

#include "DescriptorPool.hpp"

#include "engine/rendering/Device.hpp"

namespace fgl::engine
{
	DescriptorPool::DescriptorPool( Device& device, std::uint32_t set_count )
	{
		std::vector< vk::DescriptorPoolSize > pool_sizes {};
		for ( auto& [ type, ratio ] : descriptor_allocation_ratios )
		{
			pool_sizes.emplace_back( type, static_cast< std::uint32_t >( static_cast< float >( set_count ) * ratio ) );
		}

		vk::DescriptorPoolCreateInfo pool_info {};
		pool_info.setPoolSizeCount( static_cast< std::uint32_t >( pool_sizes.size() ) );
		pool_info.setPPoolSizes( pool_sizes.data() );
		pool_info.setMaxSets( set_count );
		pool_info.setFlags(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind );

		vk::Device vk_device { device.device() };
		if ( vk_device.createDescriptorPool( &pool_info, nullptr, &m_pool ) != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to create descriptor pool" );
	}

	[[nodiscard]] vk::DescriptorSet DescriptorPool::allocateSet( vk::DescriptorSetLayout& layout )
	{
		vk::DescriptorSet set;
		vk::DescriptorSetAllocateInfo alloc_info {};
		alloc_info.setDescriptorPool( m_pool );
		alloc_info.setDescriptorSetCount( 1 );
		alloc_info.setPSetLayouts( &layout );

		vk::Device vk_device { Device::getInstance().device() };
		if ( vk_device.allocateDescriptorSets( &alloc_info, &set ) != vk::Result::eSuccess )
			throw std::runtime_error( "Failed to allocate descriptor set" );

		return set;
	}

	static DescriptorPool* s_pool { nullptr };

	DescriptorPool& DescriptorPool::init( Device& device )
	{
		assert( !s_pool && "Descriptor pool already initialized" );
		s_pool = new DescriptorPool( device, 1000 );
		return *s_pool;
	}

	[[nodiscard]] DescriptorPool& DescriptorPool::getInstance()
	{
		assert( s_pool && "Descriptor pool not initialized" );
		return *s_pool;
	}

	void DescriptorPool::deallocSet( vk::DescriptorSet& set )
	{
		vk::Device vk_device { Device::getInstance().device() };
		vk_device.freeDescriptorSets( m_pool, 1, &set );
	}

} // namespace fgl::engine