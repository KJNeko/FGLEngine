//
// Created by kj16609 on 1/18/24.
//

#include "Texture.hpp"

#include <initializer_list>

#include "engine/FrameInfo.hpp"
#include "engine/assets/TransferManager.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/image/Image.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/logging/logging.hpp"
#include "engine/math/noise/perlin/generator.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "objectloaders/stb_image.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{
	using TextureID = std::uint32_t;
	static std::queue< TextureID > unused_ids {};

	TextureID getNextID()
	{
		static TextureID id { 0 };

		if ( unused_ids.size() > 0 )
		{
			const TextureID pulled_id { unused_ids.front() };
			unused_ids.pop();

			log::debug( "Gave ID {} to texture", pulled_id );

			return pulled_id;
		}
		else
		{
			return id++;
		}
	}

	std::tuple< std::vector< std::byte >, int, int, vk::Format >
		loadTexture( const std::filesystem::path& path, vk::Format format = vk::Format::eUndefined )
	{
		ZoneScoped;
		if ( !std::filesystem::exists( path ) ) throw std::runtime_error( "Failed to open file: " + path.string() );

		//TODO: More robust image loading. I should be checking what channels images have and what they are using for their bits per channel.
		if ( format == vk::Format::eUndefined ) format = vk::Format::eR8G8B8A8Unorm;

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
		if ( !ready() )
		{
			log::debug( "Unable to draw Image {}. Image not ready", this->getID() );
			return;
		}

		if ( m_imgui_set == VK_NULL_HANDLE ) createImGuiSet();

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

		if ( !ready() )
		{
			//TODO: Render placeholder
			log::warn( "Attempted to render texture {} but texture was not ready!", this->m_texture_id );
			return ImGui::Button( "No texture :(" );
		}

		assert( *m_image_view->getSampler() != VK_NULL_HANDLE );

		const ImVec2 imgui_size { static_cast< float >( extent.width ), static_cast< float >( extent.height ) };

		return ImGui::ImageButton( static_cast< ImTextureID >( getImGuiDescriptorSet() ), imgui_size );
	}

	Texture::Texture( std::tuple< std::vector< std::byte >, int, int, vk::Format > tuple ) :
	  Texture(
		  std::move( std::get< 0 >( tuple ) ), std::get< 1 >( tuple ), std::get< 2 >( tuple ), std::get< 3 >( tuple ) )
	{}

	Texture::Texture( std::vector< std::byte >&& data, const int x, const int y, const vk::Format format ) :
	  Texture( std::forward< std::vector< std::byte > >( data ), vk::Extent2D( x, y ), format )
	{}

	Texture::Texture( std::vector< std::byte >&& data, const vk::Extent2D extent, const vk::Format format ) :
	  m_texture_id( getNextID() ),
	  m_image( std::make_shared< Image >(
		  extent,
		  format,
		  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		  vk::ImageLayout::eUndefined,
		  vk::ImageLayout::eShaderReadOnlyOptimal ) ),
	  m_image_view( m_image->getView() ),
	  m_extent( extent )
	{
		memory::TransferManager::getInstance()
			.copyToImage( std::forward< std::vector< std::byte > >( data ), *m_image );

#if ENABLE_IMGUI
		createImGuiSet();
#endif
	}

	Texture::Texture( const std::filesystem::path& path, const vk::Format format ) :
	  Texture( loadTexture( path, format ) )
	{
		setName( path.filename() );
	}

	Texture::Texture( const std::filesystem::path& path ) : Texture( loadTexture( path ) )
	{
		setName( path.filename() );
	}

	Texture::~Texture()
	{
		if ( m_imgui_set != VK_NULL_HANDLE ) ImGui_ImplVulkan_RemoveTexture( m_imgui_set );
		unused_ids.push( m_texture_id );
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
		if ( !this->ready() )
		{
			log::debug( "Unable to create ImGui set. Texture was not ready" );
			return;
		}

		log::debug( "Created ImGui set for image ID {}", this->getID() );
		if ( m_imgui_set != VK_NULL_HANDLE ) return;

		auto& view { m_image_view };

		assert( view );

		VkImageView vk_view { **view };
		assert( vk_view );

		VkSampler vk_sampler { *( view->getSampler() ) };

		m_imgui_set = ImGui_ImplVulkan_AddTexture( vk_sampler, vk_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
#else
		FGL_UNREACHABLE();
#endif
	}

	vk::DescriptorSet& Texture::getImGuiDescriptorSet()
	{
		assert( ready() );
		assert( m_imgui_set != VK_NULL_HANDLE );
		return m_imgui_set;
	}

	Texture::Texture( Image& image, Sampler sampler ) :
	  m_texture_id( getNextID() ),
	  m_image(),
	  m_image_view( image.getView() ),
	  //TODO: Figure out how to get extents from images.
	  m_extent()
	{
		m_image_view->getSampler() = std::move( sampler );
	}

	bool Texture::ready() const
	{
		assert( m_image_view );
		return this->m_image_view->ready();
	}

	TextureID Texture::getID() const
	{
		return m_texture_id;
	}

	void Texture::setName( const std::string& str )
	{
		this->getImageView().setName( str );
	}

	descriptors::DescriptorSet& Texture::getTextureDescriptorSet()
	{
		static std::unique_ptr< descriptors::DescriptorSet > set { nullptr };
		static std::optional< vk::raii::DescriptorSetLayout > set_layout { std::nullopt };

		if ( set )
			return *set;
		else
		{
			set_layout = TextureDescriptorSet::createLayout();

			if ( !set_layout.has_value() ) throw std::runtime_error( "No set layout made" );

			set = std::make_unique< descriptors::DescriptorSet >( std::move( set_layout.value() ) );
			set->setMaxIDX( 1 );
			set->setName( "Texture descriptor set" );
			return *set;
		}
	}

} // namespace fgl::engine
