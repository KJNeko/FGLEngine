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

	class DescriptorSet : public std::enable_shared_from_this< DescriptorSet >
	{
		DescriptorIDX m_set_idx;
		//TODO: Maybe redo this to not be a monostate variant?
		std::vector< std::variant< std::monostate, vk::DescriptorImageInfo, vk::DescriptorBufferInfo > > m_infos;
		std::vector< vk::WriteDescriptorSet > m_descriptor_writes;
		bool m_initalized { false };

		using Resource = std::variant< std::shared_ptr< ImageView >, std::shared_ptr< memory::BufferSuballocation > >;

		//! Resources to keep allocated for as long as this descriptor exists.
		std::vector< Resource > m_resources;

		vk::raii::DescriptorSet m_set;

		std::size_t m_binding_count;

		//! Resets the binding update list
		void resetUpdate();

	  public:

		[[nodiscard]] bool hasUpdates() const;

		//! Updates the descriptor set, updates all pending writes created by using bindImage(), bindUniformBuffer(), bindArray(), bindAttachment(), or bindTexture().
		void update();

		[[nodiscard]] VkDescriptorSet operator*() const;

		[[nodiscard]] VkDescriptorSet getVkDescriptorSet() const;

		[[nodiscard]] DescriptorIDX setIDX() const { return m_set_idx; }

		FGL_DELETE_DEFAULT_CTOR( DescriptorSet );

		DescriptorSet( const vk::raii::DescriptorSetLayout& layout, DescriptorIDX idx, std::size_t binding_count );

		FGL_DELETE_COPY( DescriptorSet );

		FGL_DELETE_MOVE( DescriptorSet );

		//Move
		// DescriptorSet( DescriptorSet&& other ) noexcept;
		// DescriptorSet& operator=( DescriptorSet&& other ) noexcept;

		~DescriptorSet();

		void bindImage( std::uint32_t binding_idx, const ImageView& view, vk::ImageLayout layout );

		void bindUniformBuffer( std::uint32_t binding_idx, const memory::BufferSuballocation& buffer );
		void bindStorageBuffer( std::uint32_t binding_idx, const memory::BufferSuballocation& buffer );
		void bindArray(
			std::uint32_t binding_idx,
			const memory::BufferSuballocation& buffer,
			std::size_t array_idx,
			std::size_t item_size );

		void bindAttachment( std::uint32_t binding_idx, const ImageView& view, vk::ImageLayout layout );

		void bindTexture( std::uint32_t binding_idx, const std::shared_ptr< Texture >& tex_ptr );

		void setName( const std::string& str ) const;
	};

	using DescriptorSetPtr = std::shared_ptr< DescriptorSet >;

	//! Queues a descriptor to be deleted.
	//
	void queueDescriptorDeletion( descriptors::DescriptorSetPtr set );
	void deleteQueuedDescriptors();

} // namespace fgl::engine::descriptors