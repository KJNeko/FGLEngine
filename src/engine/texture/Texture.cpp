//
// Created by kj16609 on 1/18/24.
//

#include "Texture.hpp"

#include <initializer_list>

#include "engine/FrameInfo.hpp"
#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/image/ImageView.hpp"
#include "objectloaders/stb_image.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include "engine/math/noise/perlin/generator.hpp"
#include "imgui/backends/imgui_impl_vulkan.h"
#pragma GCC diagnostic pop

#include "engine/logging/logging.hpp"

namespace fgl::engine
{

	std::tuple< std::vector< std::byte >, int, int, vk::Format >
		loadTexture( const std::filesystem::path& path, const vk::Format format )
	{
		ZoneScoped;
		if ( !std::filesystem::exists( path ) ) throw std::runtime_error( "Failed to open file: " + path.string() );

		int x { 0 };
		int y { 0 };
		int channels { 0 };

		std::string path_str { path.string() };

		const auto data_c { stbi_load( path_str.data(), &x, &y, &channels, 4 ) };

		std::vector< std::byte > data {};

		data.resize( x * y * 4 );
		std::memcpy( data.data(), data_c, x * y * 4 );

		stbi_image_free( data_c );

		//TODO: Write check to ensure the format matches the number of channels

		return { std::move( data ), x, y, format };
	}

	void Texture::drawImGui( vk::Extent2D extent )
	{
		if ( this->m_imgui_set == VK_NULL_HANDLE ) createImGuiSet();

		if ( extent == vk::Extent2D() )
		{
			extent = getExtent();
		}

		const ImVec2 imgui_size { static_cast< float >( extent.width ), static_cast< float >( extent.height ) };

		ImGui::Image( static_cast< ImTextureID >( getImGuiDescriptorSet() ), imgui_size );
	}

	bool Texture::drawImGuiButton( vk::Extent2D extent )
	{
		if ( this->m_imgui_set == VK_NULL_HANDLE ) createImGuiSet();

		if ( extent == vk::Extent2D() )
		{
			extent = getExtent();
		}

		const ImVec2 imgui_size { static_cast< float >( extent.width ), static_cast< float >( extent.height ) };

		if ( !isReady() )
		{
			//TODO: Render placeholder
			log::warn( "Attempted to render texture {} but texture was not ready!", this->m_texture_id );
			return ImGui::Button( "No texture :(" );
		}

		return ImGui::ImageButton( static_cast< ImTextureID >( getImGuiDescriptorSet() ), imgui_size );
	}

	Texture::Texture( const std::tuple< std::vector< std::byte >, int, int, vk::Format >& tuple ) :
	  Texture( std::get< 0 >( tuple ), std::get< 1 >( tuple ), std::get< 2 >( tuple ), std::get< 3 >( tuple ) )
	{}

	Texture::Texture( const std::vector< std::byte >& data, const int x, const int y, const vk::Format format ) :
	  Texture( data, vk::Extent2D( x, y ), format )
	{}

	Texture::Texture( const std::vector< std::byte >& data, const vk::Extent2D extent, const vk::Format format ) :
	  m_extent( extent )
	{
		ZoneScoped;
		static TextureID tex_counter { 0 };

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

	Texture::Texture( const std::filesystem::path& path, const vk::Format format ) :
	  Texture( loadTexture( path, format ) )
	{}

	Texture::~Texture()
	{
		if ( m_imgui_set != VK_NULL_HANDLE ) ImGui_ImplVulkan_RemoveTexture( m_imgui_set );
	}

	void Texture::stage( vk::CommandBuffer& cmd )
	{
		ZoneScoped;

		//assert( m_staging && "Can't stage. No staging buffer made" );

		// Texutres are made with a staging buffer in RAM, Thus if the buffer has been dropped then we have been sucesfully staged.
		if ( !m_staging ) return;

		vk::ImageSubresourceRange range;
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier barrier {};
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.image = m_image_view->getVkImage();
		barrier.subresourceRange = range;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		const std::vector< vk::ImageMemoryBarrier > barriers_to { barrier };

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			{},
			barriers_to );

		vk::BufferImageCopy region {};
		region.bufferOffset = m_staging->getOffset();
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = vk::Offset3D( 0, 0, 0 );
		region.imageExtent = vk::Extent3D( m_extent, 1 );

		cmd.copyBufferToImage(
			m_staging->getVkBuffer(), m_image_view->getVkImage(), vk::ImageLayout::eTransferDstOptimal, 1, &region );

		//Transfer back to eGeneral

		vk::ImageMemoryBarrier barrier_from {};
		barrier_from.oldLayout = barrier.newLayout;
		barrier_from.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier_from.image = m_image_view->getVkImage();
		barrier_from.subresourceRange = range;
		barrier_from.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier_from.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		const std::vector< vk::ImageMemoryBarrier > barriers_from { barrier_from };

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			{},
			{},
			barriers_from );
	}

	void Texture::dropStaging()
	{
		assert( m_staging );
		m_staging.reset();
	}

	vk::DescriptorImageInfo Texture::getDescriptor() const
	{
		return m_image_view->descriptorInfo( vk::ImageLayout::eGeneral );
	}

	vk::Extent2D Texture::getExtent() const
	{
		return m_image_view->getExtent();
	}

	ImageView& Texture::getImageView()
	{
		assert( m_image_view );
		return *m_image_view;
	}

	void Texture::createImGuiSet()
	{
#if ENABLE_IMGUI
		if ( m_imgui_set != VK_NULL_HANDLE ) return;

		auto& view { m_image_view };

		assert( view );

		VkImageView vk_view { view->getVkView() };
		assert( vk_view );

		VkSampler vk_sampler { view->getSampler().getVkSampler() };

		m_imgui_set = ImGui_ImplVulkan_AddTexture( vk_sampler, vk_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
#endif
	}

	vk::DescriptorSet& Texture::getImGuiDescriptorSet()
	{
		assert( !m_staging );
		assert( m_imgui_set != VK_NULL_HANDLE );
		return m_imgui_set;
	}

	TextureID Texture::getID() const
	{
		assert( !m_staging );
		return m_texture_id;
	}

	DescriptorSet& Texture::getTextureDescriptorSet()
	{
		static std::unique_ptr< DescriptorSet > set { nullptr };
		static std::optional< vk::DescriptorSetLayout > set_layout { std::nullopt };

		if ( set )
			return *set;
		else
		{
			set_layout = TextureDescriptorSet::createLayout();
			set = std::make_unique< DescriptorSet >( set_layout.value() );
			set->setMaxIDX( 1 );
			set->setName( "Texture descriptor set" );
			return *set;
		}
	}

} // namespace fgl::engine
