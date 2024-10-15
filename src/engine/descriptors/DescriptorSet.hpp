//
// Created by kj16609 on 1/5/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <variant>

#include "engine/memory/buffers/BufferSuballocation.hpp"

namespace fgl::engine
{

	namespace memory
	{
		class BufferSuballocation;
	}

	class Texture;
	class ImageView;
} // namespace fgl::engine

namespace fgl::engine::descriptors
{

	using DescriptorIDX = std::uint32_t;

	class DescriptorSet
	{
		DescriptorIDX m_set_idx;
		//TODO: Maybe redo this to not be a monostate variant?
		std::vector< std::variant< std::monostate, vk::DescriptorImageInfo, vk::DescriptorBufferInfo > > m_infos {};
		std::vector< vk::WriteDescriptorSet > descriptor_writes {};

		std::vector< std::variant< std::shared_ptr< ImageView >, std::shared_ptr< memory::BufferSuballocation > > >
			m_resources {};

		vk::raii::DescriptorSet m_set;

		std::size_t m_binding_count;

	  public:

		void reset();

		void update();

		VkDescriptorSet operator*() const { return *m_set; }

		VkDescriptorSet getVkDescriptorSet() const { return *m_set; }

		inline DescriptorIDX setIDX() const { return m_set_idx; }

		DescriptorSet() = delete;
		DescriptorSet( const vk::raii::DescriptorSetLayout& layout, const DescriptorIDX idx, std::size_t binding_count );

		//Copy
		DescriptorSet( const DescriptorSet& other ) = delete;
		DescriptorSet& operator=( const DescriptorSet& other ) = delete;

		//Move
		DescriptorSet( DescriptorSet&& other ) noexcept;
		DescriptorSet& operator=( DescriptorSet&& other ) noexcept;

		~DescriptorSet();

		void bindImage(
			std::uint32_t binding_idx,
			ImageView& view,
			vk::ImageLayout layout,
			vk::raii::Sampler sampler = VK_NULL_HANDLE );

		void bindUniformBuffer( std::uint32_t binding_idx, memory::BufferSuballocation& buffer );
		void bindArray(
			std::uint32_t binding_idx,
			const memory::BufferSuballocation& buffer,
			std::size_t array_idx,
			std::size_t item_size );

		void bindAttachment(
			std::uint32_t binding_idx,
			const ImageView& view,
			vk::ImageLayout layout,
			const vk::raii::Sampler& sampler = VK_NULL_HANDLE );

		void bindTexture( std::uint32_t binding_idx, std::shared_ptr< Texture >& tex_ptr );

		void setName( const std::string& str );
	};

	//! Queues a descriptor to be deleted.
	//
	void queueDescriptorDeletion( std::unique_ptr< DescriptorSet > set );
	void deleteQueuedDescriptors();

} // namespace fgl::engine::descriptors