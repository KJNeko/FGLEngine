//
// Created by kj16609 on 1/18/24.
//

#include "Texture.hpp"

#include <initializer_list>

#include "engine/FrameInfo.hpp"
#include "engine/assets/image/Image.hpp"
#include "engine/assets/image/ImageView.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/math/noise/perlin/generator.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include "engine/utility/IDPool.hpp"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "objectloaders/stb_image.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{

	static IDPool< TextureID > texture_id_pool { 1 };

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

		ImGui::Image( getImGuiDescriptorSet(), imgui_size );
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

		return ImGui::ImageButton( m_name.c_str(), getImGuiDescriptorSet(), imgui_size );
	}

	Texture::Texture( std::tuple< std::vector< std::byte >, int, int, vk::Format > tuple ) :
	  Texture(
		  std::move( std::get< 0 >( tuple ) ), std::get< 1 >( tuple ), std::get< 2 >( tuple ), std::get< 3 >( tuple ) )
	{}

	Texture::Texture( std::vector< std::byte >&& data, const int x, const int y, const vk::Format format ) :
	  Texture( std::forward< std::vector< std::byte > >( data ), vk::Extent2D( x, y ), format )
	{}

	Texture::Texture( std::vector< std::byte >&& data, const vk::Extent2D extent, const vk::Format format ) :
	  m_texture_id( texture_id_pool.getID() ),
	  m_image( std::make_shared< Image >(
		  extent,
		  format,
		  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		  vk::ImageLayout::eUndefined,
		  vk::ImageLayout::eShaderReadOnlyOptimal ) ),
	  m_image_view( m_image->getView() ),
	  m_extent( extent ),
	  m_name( "Default Texture Name" )
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
		texture_id_pool.markUnused( m_texture_id );
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
	}

	vk::DescriptorSet& Texture::getImGuiDescriptorSet()
	{
		assert( ready() );
		assert( m_imgui_set != VK_NULL_HANDLE );
		return m_imgui_set;
	}

	Texture::Texture( Image& image, Sampler sampler ) :
	  m_texture_id( texture_id_pool.getID() ),
	  m_image(),
	  m_image_view( image.getView() ),
	  //TODO: Figure out how to get extents from images.
	  m_extent(),
	  m_name( "Default Texture Name" )
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
		m_image->setName( str + " Image" );
		m_image_view->setName( str + " ImageView" );
	}

	inline static descriptors::DescriptorSetLayout texture_descriptor_set { TEXTURE_SET_ID, texture_descriptor };

	descriptors::DescriptorSetLayout& Texture::getDescriptorLayout()
	{
		return texture_descriptor_set;
	}

	descriptors::DescriptorSet& Texture::getDescriptorSet()
	{
		static std::unique_ptr< descriptors::DescriptorSet > set { nullptr };

		if ( set )
			return *set;
		else
		{
			set = texture_descriptor_set.create();
			set->setName( "Texture descriptor set" );
			return *set;
		}
	}

} // namespace fgl::engine
