//
// Created by kj16609 on 1/5/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <variant>

#include "engine/buffers/Buffer.hpp"
#include "engine/buffers/BufferSuballocation.hpp"

namespace fgl::engine
{

	class BufferSuballocation;
	class Texture;
	class ImageView;

	class DescriptorSet
	{
		std::vector< std::variant< std::monostate, vk::DescriptorImageInfo, vk::DescriptorBufferInfo > > m_infos {};
		std::vector< vk::WriteDescriptorSet > descriptor_writes {};

		std::vector< std::variant< std::shared_ptr< ImageView >, std::shared_ptr< BufferSuballocation > > > m_resources;

		vk::DescriptorSetLayout m_layout;
		vk::DescriptorSet m_set;

		std::uint32_t m_max_idx { 0 };

	  public:

		void reset();

		void update();

		void setMaxIDX( std::uint32_t max_idx );

		vk::DescriptorSet& getSet() { return m_set; }

		DescriptorSet() = delete;
		DescriptorSet( vk::DescriptorSetLayout layout );
		~DescriptorSet();

		//Copy
		DescriptorSet( const DescriptorSet& other ) = delete;
		DescriptorSet& operator=( const DescriptorSet& other ) = delete;

		//Move
		DescriptorSet( DescriptorSet&& other );
		DescriptorSet& operator=( DescriptorSet&& other );

		void bindImage(
			std::uint32_t binding_idx, ImageView& view, vk::ImageLayout layout, vk::Sampler sampler = VK_NULL_HANDLE );

		void bindUniformBuffer( std::uint32_t binding_idx, BufferSuballocation& buffer );

		void bindAttachment(
			std::uint32_t binding_idx, ImageView& view, vk::ImageLayout layout, vk::Sampler sampler = VK_NULL_HANDLE );

		void bindTexture( std::uint32_t binding_idx, Texture& tex );

		void setName( const std::string str );
	};

} // namespace fgl::engine