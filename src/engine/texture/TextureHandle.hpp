//
// Created by kj16609 on 1/22/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory>

namespace fgl::engine
{
	class ImageView;
	class BufferSuballocation;

	using TextureID = std::uint32_t;

	class TextureHandle
	{
		//TODO: Implement reusing texture ids
		TextureID m_texture_id { std::numeric_limits< TextureID >::infinity() };

		std::shared_ptr< ImageView > m_image_view {};

		std::unique_ptr< BufferSuballocation > m_staging { nullptr };

		vk::Extent2D m_extent { 0, 0 };

		vk::DescriptorSet m_imgui_set { VK_NULL_HANDLE };

		friend class Texture;

	  public:

		TextureHandle( const std::vector< unsigned char >& data, const vk::Extent2D extent, const int channels );
		~TextureHandle();
	};

} // namespace fgl::engine
