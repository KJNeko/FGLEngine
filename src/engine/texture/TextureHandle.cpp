//
// Created by kj16609 on 1/22/24.
//

#include "TextureHandle.hpp"

#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/image/Image.hpp"
#include "engine/image/ImageView.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include "imgui/backends/imgui_impl_vulkan.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{

	TextureHandle::TextureHandle(
		const std::vector< std::byte >& data, const vk::Extent2D extent, [[maybe_unused]] const int channels ) :
	  m_extent( extent )
	{
		ZoneScoped;
		static TextureID tex_counter { 0 };

		constexpr auto format { vk::Format::eR8G8B8A8Unorm };

		auto image = std::make_shared< Image >(
			extent,
			format,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal );

		m_image_view = image->getView();

		m_texture_id = tex_counter++;

		m_staging = std::make_unique< BufferSuballocation >( getGlobalStagingBuffer(), data.size() );
		//Copy data info buffer
		std::memcpy( reinterpret_cast< unsigned char* >( m_staging->ptr() ), data.data(), data.size() );
	}

	TextureHandle::~TextureHandle()
	{
		if ( m_imgui_set != VK_NULL_HANDLE ) ImGui_ImplVulkan_RemoveTexture( m_imgui_set );
	}

} // namespace fgl::engine
