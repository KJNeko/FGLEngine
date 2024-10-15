//
// Created by kj16609 on 1/5/24.
//

#include "DescriptorSet.hpp"

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <queue>

#include "DescriptorPool.hpp"
#include "engine/assets/image/ImageView.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/memory/buffers/BufferSuballocation.hpp"
#include "engine/rendering/SwapChain.hpp"

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
	  descriptor_writes( std::move( other.descriptor_writes ) ),
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
		descriptor_writes = std::move( other.descriptor_writes );
		m_resources = std::move( other.m_resources );
		m_set = std::move( other.m_set );
		other.m_set = VK_NULL_HANDLE;
		m_binding_count = other.m_binding_count;
		other.m_binding_count = 0;

		return *this;
	}

	void DescriptorSet::bindUniformBuffer( const std::uint32_t binding_idx, memory::BufferSuballocation& buffer )
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
		write.dstArrayElement = array_idx;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eUniformBuffer;
		write.pBufferInfo = &( std::get< vk::DescriptorBufferInfo >( m_infos.data()[ binding_idx ] ) );
		write.pImageInfo = VK_NULL_HANDLE;
		write.pTexelBufferView = VK_NULL_HANDLE;

		log::info( "Bound idx {} to data", array_idx );

		descriptor_writes.push_back( write );
	}

	void DescriptorSet::
		bindImage( std::uint32_t binding_idx, ImageView& view, vk::ImageLayout layout, vk::raii::Sampler sampler )
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

	void DescriptorSet::bindTexture( std::uint32_t binding_idx, std::shared_ptr< Texture >& tex_ptr )
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
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos.data()[ binding_idx ] ) );

		descriptor_writes.push_back( write );
	}

	void DescriptorSet::update()
	{
		Device::getInstance().device().updateDescriptorSets( descriptor_writes, {} );
		reset();
	}

	DescriptorSet::~DescriptorSet()
	{}

	void DescriptorSet::reset()
	{
		m_infos.clear();
		//Clear all writes
		descriptor_writes.clear();

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
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos.data()[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		descriptor_writes.push_back( write );
	}

	void DescriptorSet::setName( const std::string& str )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eDescriptorSet;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< std::uint64_t >( getVkDescriptorSet() ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

	inline static std::vector< std::pair< std::uint_fast8_t, std::unique_ptr< DescriptorSet > > > queue {};

	void queueDescriptorDeletion( std::unique_ptr< DescriptorSet > set )
	{
		queue.emplace_back( std::make_pair( 0, std::move( set ) ) );
	}

	void deleteQueuedDescriptors()
	{
		for ( auto itter = queue.begin(); itter != queue.end(); itter = itter++ )
		{
			auto& [ counter, set ] = *itter;
			// Prevent deleting a descriptor until we are sure it's been here long enough
			if ( counter > SwapChain::MAX_FRAMES_IN_FLIGHT + 1 )
			{
				itter = queue.erase( itter );
			}
			else
				++counter;
		}
	}

} // namespace fgl::engine::descriptors