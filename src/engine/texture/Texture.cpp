//
// Created by kj16609 on 1/18/24.
//

#include "Texture.hpp"

#include <initializer_list>

#include "TextureHandle.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/image/ImageView.hpp"
#include "objectloaders/stb_image.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include "imgui/imgui_impl_vulkan.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{

	inline static std::unordered_map< std::string, std::weak_ptr< TextureHandle > > texture_map;

	std::tuple< std::vector< unsigned char >, int, int, int > loadTexture( const std::filesystem::path& path )
	{
		ZoneScoped;
		if ( !std::filesystem::exists( path ) ) throw std::runtime_error( "Failed to open file: " + path.string() );

		int x { 0 };
		int y { 0 };
		int channels { 0 };

		std::string path_str { path.string() };

		const auto data_c { stbi_load( path_str.data(), &x, &y, &channels, 4 ) };

		std::cout << "Loaded image with " << x << "x" << y << " Image had " << channels << " channels" << std::endl;

		std::vector< unsigned char > data {};

		data.resize( x * y * 4 );
		std::memcpy( data.data(), data_c, x * y * 4 );

		stbi_image_free( data_c );

		return { std::move( data ), x, y, 4 };
	}

	Texture Texture::loadFromFile( const std::filesystem::path& path )
	{
		ZoneScoped;
		//TODO: Make some way of cleaning the map when loading textures

		if ( texture_map.contains( path.string() ) )
		{
			if ( auto itter = texture_map.find( path.string() ); !itter->second.expired() )
			{
				return Texture( itter->second.lock() );
			}
			else
			{
				//Texture is expired. So it'll need to be reloaded.
				texture_map.erase( itter );
			}
		}

		auto data { loadTexture( path ) };
		Texture tex { data };
		tex.m_handle->m_image_view->setName( path.string() );

		texture_map.emplace( path.string(), tex.m_handle );

		return tex;
	}

	Texture::Texture( std::shared_ptr< TextureHandle > handle ) : m_handle( handle )
	{}

	Texture::Texture( std::tuple< std::vector< unsigned char >, int, int, int > tuple ) :
	  Texture( std::get< 0 >( tuple ), std::get< 1 >( tuple ), std::get< 2 >( tuple ), std::get< 3 >( tuple ) )
	{}

	Texture::Texture( std::vector< unsigned char >& data, const int x, const int y, const int channels ) :
	  Texture( data, vk::Extent2D( x, y ), channels )
	{}

	Texture::Texture( std::vector< unsigned char >& data, const vk::Extent2D extent, const int channels ) :
	  m_handle( std::make_shared< TextureHandle >( data, extent, channels ) )
	{}

	void Texture::stage( vk::CommandBuffer& cmd )
	{
		ZoneScoped;
		assert( m_handle && "Attempted to stage invalid texture (No handle)" );

		//assert( m_handle->m_staging && "Can't stage. No staging buffer made" );

		//TODO: I need some way of telling if a Texture HAS been staged rather then simply checking if the staging buffer is present
		// Since just checking if the buffer is present could not mean it has been staged (IE staged but not dropped, Or never created in the first place)
		if ( !m_handle->m_staging ) return;

		vk::ImageSubresourceRange range;
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier barrier {};
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.image = m_handle->m_image_view->getVkImage();
		barrier.subresourceRange = range;
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		std::vector< vk::ImageMemoryBarrier > barriers_to { barrier };

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			{},
			barriers_to );

		vk::BufferImageCopy region {};
		region.bufferOffset = m_handle->m_staging->getOffset();
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = vk::Offset3D( 0, 0, 0 );
		region.imageExtent = vk::Extent3D( m_handle->m_extent, 1 );

		cmd.copyBufferToImage(
			m_handle->m_staging->getVkBuffer(),
			m_handle->m_image_view->getVkImage(),
			vk::ImageLayout::eTransferDstOptimal,
			1,
			&region );

		//Transfer back to eGeneral

		vk::ImageMemoryBarrier barrier_from {};
		barrier_from.oldLayout = barrier.newLayout;
		barrier_from.newLayout = vk::ImageLayout::eGeneral;
		barrier_from.image = m_handle->m_image_view->getVkImage();
		barrier_from.subresourceRange = range;
		barrier_from.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier_from.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		std::vector< vk::ImageMemoryBarrier > barriers_from { barrier_from };

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
		m_handle->m_staging.reset();
	}

	vk::DescriptorImageInfo Texture::getDescriptor() const
	{
		return m_handle->m_image_view->descriptorInfo( vk::ImageLayout::eGeneral );
	}

	vk::Extent2D Texture::getExtent() const
	{
		return m_handle->m_image_view->getExtent();
	}

	ImageView& Texture::getImageView()
	{
		assert( m_handle );
		assert( m_handle->m_image_view );
		return *m_handle->m_image_view;
	}

	void Texture::createImGuiSet()
	{
#ifdef IMGUI_ENABLE
		if ( m_handle->m_imgui_set != VK_NULL_HANDLE ) return;

		auto& view { m_handle->m_image_view };

		VkImageView vk_view { view->getVkView() };
		VkSampler vk_sampler { view->getSampler()->getVkSampler() };

		m_handle->m_imgui_set = ImGui_ImplVulkan_AddTexture( vk_sampler, vk_view, VK_IMAGE_LAYOUT_GENERAL );
#endif
	}

	vk::DescriptorSet& Texture::getImGuiDescriptorSet()
	{
		return m_handle->m_imgui_set;
	}

	TextureID Texture::getID() const
	{
		return m_handle->m_texture_id;
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
