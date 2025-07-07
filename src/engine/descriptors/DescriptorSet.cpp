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
#include "memory/buffers/BufferSuballocationHandle.hpp"

namespace fgl::engine::descriptors
{

	DescriptorSet::DescriptorSet(
		const vk::raii::DescriptorSetLayout& layout, const DescriptorIDX idx, const std::size_t binding_count ) :
	  m_binding_count( binding_count ),
	  m_set_idx( idx ),
	  m_infos( m_binding_count ),
	  m_descriptor_writes(),
	  m_resources( m_binding_count ),
	  m_set( DescriptorPool::getInstance().allocateSet( layout ) )
	{
		// m_resources.resize( m_binding_count );
		// m_infos.resize( m_binding_count );
	}

	void DescriptorSet::
		bindBuffer( const std::uint32_t binding_idx, const memory::SwappableBufferSuballocation& buffer_suballocation )
	{
		return bindBuffer( binding_idx, buffer_suballocation.freeze() );
	}

	void DescriptorSet::
		bindBuffer( const std::uint32_t binding_idx, const memory::FrozenBufferSuballocation& buffer_suballocation )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );
		if ( buffer_suballocation->bytesize() == 0 ) return;

		m_infos[ binding_idx ] = buffer_suballocation->getDescriptorInfo();

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = buffer_suballocation->getDescriptorType();
		write.pBufferInfo = &( std::get< vk::DescriptorBufferInfo >( m_infos[ binding_idx ] ) );
		write.pImageInfo = VK_NULL_HANDLE;
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_resources[ binding_idx ] = buffer_suballocation;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindArray(
		const std::uint32_t binding_idx,
		memory::SwappableBufferSuballocation& buffer_suballocation,
		const std::size_t array_idx,
		const std::size_t item_size )
	{
		return bindArray( binding_idx, buffer_suballocation.freeze(), array_idx, item_size );
	}

	void DescriptorSet::bindArray(
		const std::uint32_t binding_idx,
		const memory::FrozenBufferSuballocation& buffer_suballocation,
		const std::size_t array_idx,
		const std::size_t item_size )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		m_infos[ binding_idx ] = buffer_suballocation->getDescriptorInfo( array_idx, item_size );

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

		m_resources[ binding_idx ] = buffer_suballocation;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindImage( BindingIDX binding_idx, const ImageViewPtr& view, vk::ImageLayout layout )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		//Store info
		m_infos[ binding_idx ] = view->descriptorInfo( layout );

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eSampledImage;
		write.pBufferInfo = VK_NULL_HANDLE;
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_resources[ binding_idx ] = view;

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

		m_resources[ binding_idx ] = tex_ptr;

		m_descriptor_writes.push_back( write );
	}

	void DescriptorSet::bindAttachment( BindingIDX binding_idx, const ImageViewPtr& view, vk::ImageLayout layout )
	{
		assert( binding_idx < m_infos.size() && "Binding index out of range" );

		//Store info
		m_infos[ binding_idx ] = view->descriptorInfo( layout );

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eInputAttachment;
		write.pBufferInfo = VK_NULL_HANDLE;
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos[ binding_idx ] ) );
		write.pTexelBufferView = VK_NULL_HANDLE;

		m_resources[ binding_idx ] = view;

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

	void DescriptorSet::setName( const std::string& str ) const
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eDescriptorSet;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< std::uint64_t >( static_cast< VkDescriptorSet >( *m_set ) ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

} // namespace fgl::engine::descriptors