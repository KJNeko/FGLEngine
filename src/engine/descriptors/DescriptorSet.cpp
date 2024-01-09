//
// Created by kj16609 on 1/5/24.
//

#include "DescriptorSet.hpp"

#include <vulkan/vulkan.hpp>

#include "DescriptorPool.hpp"
#include "engine/buffers/BufferSuballocation.hpp"

namespace fgl::engine
{

	DescriptorSet::DescriptorSet( vk::DescriptorSetLayout layout ) :
	  m_layout( layout ),
	  m_set( DescriptorPool::getInstance().allocateSet( layout ) )
	{}

	DescriptorSet::~DescriptorSet()
	{
		DescriptorPool::getInstance().deallocSet( m_set );
	}

	void DescriptorSet::bindUniformBuffer( std::uint32_t binding_idx, BufferSuballocation& buffer )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		m_infos[ binding_idx ] = buffer.descriptorInfo();

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &( std::get< vk::DescriptorBufferInfo >( m_infos.data()[ binding_idx ] ) );
		write.pImageInfo = VK_NULL_HANDLE;
		write.pTexelBufferView = VK_NULL_HANDLE;

		descriptor_writes.push_back( write );
	}

	void DescriptorSet::
		bindImage( std::uint32_t binding_idx, ImageView& view, vk::ImageLayout layout, vk::Sampler sampler )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		//Store info
		m_infos[ binding_idx ] = view.descriptorInfo( sampler, layout );

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eSampledImage;
		write.pBufferInfo = VK_NULL_HANDLE;
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos.data()[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		descriptor_writes.push_back( write );
	}

	void DescriptorSet::update()
	{
		Device::getInstance().device().updateDescriptorSets( descriptor_writes, {} );
	}

	void DescriptorSet::reset()
	{
		m_infos.clear();
		//Clear all writes
		descriptor_writes.clear();

		setMaxIDX( m_max_idx );
	}

	void DescriptorSet::setMaxIDX( std::uint32_t max_idx )
	{
		m_max_idx = max_idx;
		m_infos.resize( max_idx + 1 );
	}

	void DescriptorSet::
		bindAttachment( std::uint32_t binding_idx, ImageView& view, vk::ImageLayout layout, vk::Sampler sampler )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		//Store info
		m_infos[ binding_idx ] = view.descriptorInfo( sampler, layout );

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eInputAttachment;
		write.pBufferInfo = VK_NULL_HANDLE;
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos.data()[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		descriptor_writes.push_back( write );
	}
} // namespace fgl::engine