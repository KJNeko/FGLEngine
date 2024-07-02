//
// Created by kj16609 on 1/5/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <variant>

#include "engine/buffers/BufferSuballocation.hpp"

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

	class DescriptorSet
	{
		std::vector< std::variant< std::monostate, vk::DescriptorImageInfo, vk::DescriptorBufferInfo > > m_infos {};
		std::vector< vk::WriteDescriptorSet > descriptor_writes {};

		std::vector< std::variant< std::shared_ptr< ImageView >, std::shared_ptr< memory::BufferSuballocation > > >
			m_resources {};

		vk::raii::DescriptorSetLayout m_layout;
		vk::raii::DescriptorSet m_set;

		std::uint32_t m_max_idx { 0 };

	  public:

		void reset();

		void update();

		void setMaxIDX( std::uint32_t max_idx );

		VkDescriptorSet operator*() { return *m_set; }

		VkDescriptorSet getVkDescriptorSet() { return *m_set; }

		DescriptorSet() = delete;
		DescriptorSet( vk::raii::DescriptorSetLayout&& layout );

		//Copy
		DescriptorSet( const DescriptorSet& other ) = delete;
		DescriptorSet& operator=( const DescriptorSet& other ) = delete;

		//Move
		DescriptorSet( DescriptorSet&& other );
		DescriptorSet& operator=( DescriptorSet&& other );

		void bindImage(
			std::uint32_t binding_idx,
			ImageView& view,
			vk::ImageLayout layout,
			vk::raii::Sampler sampler = VK_NULL_HANDLE );

		void bindUniformBuffer( std::uint32_t binding_idx, memory::BufferSuballocation& buffer );

		void bindAttachment(
			std::uint32_t binding_idx,
			ImageView& view,
			vk::ImageLayout layout,
			vk::raii::Sampler sampler = VK_NULL_HANDLE );

		void bindTexture( std::uint32_t binding_idx, std::shared_ptr< Texture >& tex_ptr );

		void setName( const std::string str );
	};

} // namespace fgl::engine::descriptors