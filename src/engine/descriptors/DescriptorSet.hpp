//
// Created by kj16609 on 1/5/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <variant>

#include "assets/image/ImageView.hpp"
#include "assets/texture/Texture.hpp"
#include "engine/memory/buffers/BufferSuballocation.hpp"
#include "memory/buffers/BufferHandle.hpp"

namespace fgl::engine
{
	class Texture;
	class ImageView;
} // namespace fgl::engine

namespace fgl::engine::descriptors
{
	using DescriptorIDX = std::uint32_t;
	using BindingIDX = std::uint32_t;
	constexpr DescriptorIDX MAX_BINDINGS { 64 };

	class DescriptorSet : public std::enable_shared_from_this< DescriptorSet >
	{
		std::size_t m_binding_count;
		DescriptorIDX m_set_idx;
		//TODO: Maybe redo this to not be a monostate variant?
		std::vector< std::variant< std::monostate, vk::DescriptorImageInfo, vk::DescriptorBufferInfo > > m_infos;
		std::vector< vk::WriteDescriptorSet > m_descriptor_writes;
		bool m_initalized { false };

		using Resource = std::variant< std::monostate, ImageViewPtr, memory::FrozenBufferSuballocation, TexturePtr >;

		//! Resources to keep allocated for as long as this descriptor exists.
		std::vector< Resource > m_resources;

		vk::raii::DescriptorSet m_set;

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
		void bindBuffer( std::uint32_t binding_idx, const memory::SwappableBufferSuballocation& buffer_suballocation );

		FGL_DELETE_COPY( DescriptorSet );

		FGL_DELETE_MOVE( DescriptorSet );

		//Move
		// DescriptorSet( DescriptorSet&& other ) noexcept;
		// DescriptorSet& operator=( DescriptorSet&& other ) noexcept;

		~DescriptorSet();

		void setName( const std::string& str ) const;

		void bindImage( BindingIDX binding_idx, const ImageViewPtr& view, vk::ImageLayout layout );
		void bindBuffer( std::uint32_t binding_idx, const memory::FrozenBufferSuballocation& buffer_suballocation );
		void bindArray(
			std::uint32_t binding_idx,
			memory::SwappableBufferSuballocation& buffer_suballocation,
			std::size_t array_idx,
			std::size_t item_size );
		void bindArray(
			std::uint32_t binding_idx,
			const memory::FrozenBufferSuballocation& buffer_suballocation,
			std::size_t array_idx,
			std::size_t item_size );
		void bindAttachment( BindingIDX binding_idx, const ImageViewPtr& view, vk::ImageLayout layout );
		void bindTexture( BindingIDX binding_idx, const TexturePtr& tex_ptr );
	};

	using DescriptorSetPtr = std::shared_ptr< DescriptorSet >;

} // namespace fgl::engine::descriptors