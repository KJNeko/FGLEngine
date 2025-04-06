//
// Created by kj16609 on 1/5/24.
//

#include "DescriptorSet.hpp"

#include <vulkan/vulkan.hpp>

#include <iostream>

#include "DescriptorPool.hpp"
#include "engine/assets/image/ImageView.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/memory/buffers/BufferSuballocation.hpp"
#include "engine/rendering/PresentSwapChain.hpp"

namespace fgl::engine::descriptors
{

	DescriptorSet::DescriptorSet(
		const vk::raii::DescriptorSetLayout& layout, const DescriptorIDX idx, const std::size_t binding_count ) :
	  m_set_idx( idx ),
	  m_set( DescriptorPool::getInstance().allocateSet( layout ) ),
	  m_binding_count( binding_count )
	{
		m_infos.resize( m_binding_count );
	}

	DescriptorSet::DescriptorSet( DescriptorSet&& other ) noexcept :
	  m_set_idx( other.m_set_idx ),
	  m_infos( std::move( other.m_infos ) ),
	  m_descriptor_writes( std::move( other.m_descriptor_writes ) ),
	  m_resources( std::move( other.m_resources ) ),
	  m_set( std::move( other.m_set ) ),
	  m_binding_count( other.m_binding_count )
	{
		other.m_set = VK_NULL_HANDLE;
		other.m_binding_count = 0;
	}

	DescriptorSet& DescriptorSet::operator=( DescriptorSet&& other ) noexcept
	{
		m_set_idx = other.m_set_idx;
		m_infos = std::move( other.m_infos );
		m_descriptor_writes = std::move( other.m_descriptor_writes );
		m_resources = std::move( other.m_resources );
		m_set = std::move( other.m_set );
		other.m_set = VK_NULL_HANDLE;
		m_binding_count = other.m_binding_count;
		other.m_binding_count = 0;

		return *this;
	}

	void DescriptorSet::bindUniformBuffer( const std::uint32_t binding_idx, const memory::BufferSuballocation& buffer )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );
		if ( buffer.bytesize() == 0 ) return;

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

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindStorageBuffer( const std::uint32_t binding_idx, const memory::BufferSuballocation& buffer )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );
		if ( buffer.bytesize() == 0 ) return;

		m_infos[ binding_idx ] = buffer.descriptorInfo();

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eStorageBuffer;
		write.pBufferInfo = &( std::get< vk::DescriptorBufferInfo >( m_infos[ binding_idx ] ) );
		write.pImageInfo = VK_NULL_HANDLE;
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindArray(
		const std::uint32_t binding_idx,
		const memory::BufferSuballocation& buffer,
		const std::size_t array_idx,
		const std::size_t item_size )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		m_infos[ binding_idx ] = buffer.descriptorInfo( array_idx * item_size );

		//HACK: We set the range to something else after getting it
		std::get< vk::DescriptorBufferInfo >( m_infos[ binding_idx ] ).range = item_size;

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = static_cast< std::uint32_t >( array_idx );
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &( std::get< vk::DescriptorBufferInfo >( m_infos[ binding_idx ] ) );
		write.pImageInfo = VK_NULL_HANDLE;
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindImage(
		const std::uint32_t binding_idx,
		const ImageView& view,
		const vk::ImageLayout layout,
		const vk::raii::Sampler& sampler )
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
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindTexture( const std::uint32_t binding_idx, const std::shared_ptr< Texture >& tex_ptr )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );
		assert(
			std::holds_alternative< std::monostate >( m_infos[ binding_idx ] )
			&& "Update must be called between each array bind" );

		assert( tex_ptr );

		//TODO: Bind temporary texture if tex_ptr is not ready.

		Texture& tex { *tex_ptr };

		m_infos[ binding_idx ] = tex.getImageView().descriptorInfo(
			tex.getImageView().getSampler().getVkSampler(), vk::ImageLayout::eShaderReadOnlyOptimal );

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = tex.getID();
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos[ binding_idx ] ) );

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::update()
	{
		Device::getInstance().device().updateDescriptorSets( m_descriptor_writes, {} );
		resetUpdate();
	}

	DescriptorSet::~DescriptorSet()
	{}

	void DescriptorSet::resetUpdate()
	{
		m_infos.clear();
		//Clear all writes
		m_descriptor_writes.clear();

		m_infos.resize( m_binding_count );
	}

	void DescriptorSet::bindAttachment(
		const std::uint32_t binding_idx,
		const ImageView& view,
		const vk::ImageLayout layout,
		const vk::raii::Sampler& sampler )
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
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::setName( const std::string& str ) const
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eDescriptorSet;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< std::uint64_t >( getVkDescriptorSet() ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

	inline static std::vector< std::pair< std::uint_fast8_t, std::unique_ptr< DescriptorSet > > > QUEUE {};

	void queueDescriptorDeletion( std::unique_ptr< DescriptorSet > set )
	{
		QUEUE.emplace_back( 0, std::move( set ) );
	}

	void deleteQueuedDescriptors()
	{
		for ( auto itter = QUEUE.begin(); itter != QUEUE.end(); itter = itter++ )
		{
			auto& [ counter, set ] = *itter;
			// Prevent deleting a descriptor until we are sure it's been here long enough
			if ( counter > constants::MAX_FRAMES_IN_FLIGHT + 1 )
			{
				itter = QUEUE.erase( itter );
			}
			else
				++counter;
		}
	}

} // namespace fgl::engine::descriptors