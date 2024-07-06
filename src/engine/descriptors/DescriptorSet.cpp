//
// Created by kj16609 on 1/5/24.
//

#include "DescriptorSet.hpp"

#include <vulkan/vulkan.hpp>

#include <iostream>

#include "DescriptorPool.hpp"
#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine::descriptors
{

	DescriptorSet::DescriptorSet( vk::raii::DescriptorSetLayout&& layout ) :
	  m_layout( std::forward< vk::raii::DescriptorSetLayout >( layout ) ),
	  m_set( DescriptorPool::getInstance().allocateSet( m_layout ) )
	{}

	DescriptorSet::DescriptorSet( DescriptorSet&& other ) noexcept :
	  m_infos( std::move( other.m_infos ) ),
	  descriptor_writes( std::move( other.descriptor_writes ) ),
	  m_resources( std::move( other.m_resources ) ),
	  m_layout( std::move( other.m_layout ) ),
	  m_set( std::move( other.m_set ) ),
	  m_max_idx( other.m_max_idx )
	{
		other.m_set = VK_NULL_HANDLE;
	}

	DescriptorSet& DescriptorSet::operator=( DescriptorSet&& other ) noexcept
	{
		m_infos = std::move( other.m_infos );
		descriptor_writes = std::move( other.descriptor_writes );
		m_resources = std::move( other.m_resources );
		m_layout = std::move( other.m_layout );
		m_set = std::move( other.m_set );
		other.m_set = VK_NULL_HANDLE;
		m_max_idx = other.m_max_idx;
		return *this;
	}

	void DescriptorSet::bindUniformBuffer( std::uint32_t binding_idx, memory::BufferSuballocation& buffer )
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

		log::info( "Bound texture {} to global texture array", tex.getID() );

		vk::WriteDescriptorSet write {};
		write.dstSet = m_set;
		write.dstBinding = binding_idx;
		write.dstArrayElement = tex.getID();
		write.descriptorCount = 1;
		write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		write.pImageInfo = &( std::get< vk::DescriptorImageInfo >( m_infos.data()[ binding_idx ] ) );

		std::cout << "Bound texture: " << tex.getID() << std::endl;

		descriptor_writes.push_back( write );
	}

	void DescriptorSet::update()
	{
		Device::getInstance().device().updateDescriptorSets( descriptor_writes, {} );
		reset();
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
} // namespace fgl::engine::descriptors