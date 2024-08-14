//
// Created by kj16609 on 12/14/23.
//

#include "DescriptorPool.hpp"

#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine::descriptors
{

	vk::raii::DescriptorPool createPool( std::uint32_t set_count )
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

		return Device::getInstance()->createDescriptorPool( pool_info );
	}

	DescriptorPool::DescriptorPool( std::uint32_t set_count ) : m_pool( createPool( set_count ) )
	{}

	[[nodiscard]] vk::raii::DescriptorSet DescriptorPool::allocateSet( vk::raii::DescriptorSetLayout& layout )
	{
		vk::DescriptorSetAllocateInfo alloc_info {};
		alloc_info.setDescriptorPool( m_pool );
		alloc_info.setDescriptorSetCount( 1 );
		alloc_info.setPSetLayouts( &( *layout ) );

		std::vector< vk::raii::DescriptorSet > sets { Device::getInstance()->allocateDescriptorSets( alloc_info ) };
		assert( sets.size() == 1 );

		vk::raii::DescriptorSet set { std::move( sets[ 0 ] ) };

		return set;
	}

	static DescriptorPool* s_pool { nullptr };

	DescriptorPool& DescriptorPool::init()
	{
		assert( !s_pool && "Descriptor pool already initialized" );
		s_pool = new DescriptorPool( 1000 );
		return *s_pool;
	}

	[[nodiscard]] DescriptorPool& DescriptorPool::getInstance()
	{
		assert( s_pool && "Descriptor pool not initialized" );
		return *s_pool;
	}

} // namespace fgl::engine::descriptors