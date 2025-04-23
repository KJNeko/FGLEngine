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
		write.pBufferInfo = &( std::get< vk::DescriptorBufferInfo >( m_infos[ binding_idx ] ) );
		write.pImageInfo = VK_NULL_HANDLE;
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_descriptor_writes.push_back( write );

		buffer.setRebindInfoUniform( this->shared_from_this(), binding_idx );
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

		buffer.setRebindInfoStorage( this->shared_from_this(), binding_idx );
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

		buffer.setRebindInfoArray( binding_idx, this->shared_from_this(), array_idx, item_size );
	}

	void DescriptorSet::
		bindImage( const std::uint32_t binding_idx, const ImageView& view, const vk::ImageLayout layout )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		//Store info
		m_infos[ binding_idx ] = view.descriptorInfo( layout );

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

		m_infos[ binding_idx ] = tex.getImageView().descriptorInfo( vk::ImageLayout::eShaderReadOnlyOptimal );

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
		m_initalized = true;
		resetUpdate();
	}

	VkDescriptorSet DescriptorSet::operator*() const
	{
		return getVkDescriptorSet();
	}

	VkDescriptorSet DescriptorSet::getVkDescriptorSet() const
	{
		FGL_ASSERT( !hasUpdates(), "Descriptor set has updates but binding was attempted" );
		FGL_ASSERT( m_initalized, "Descriptor set has not been initialized" );
		return *m_set;
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

	bool DescriptorSet::hasUpdates() const
	{
		return !m_descriptor_writes.empty();
	}

	void DescriptorSet::
		bindAttachment( const std::uint32_t binding_idx, const ImageView& view, const vk::ImageLayout layout )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		//Store info
		m_infos[ binding_idx ] = view.descriptorInfo( layout );

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
		info.setObjectHandle( reinterpret_cast< std::uint64_t >( static_cast< VkDescriptorSet >( *m_set ) ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

} // namespace fgl::engine::descriptors