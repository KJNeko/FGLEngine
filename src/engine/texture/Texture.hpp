//
// Created by kj16609 on 1/18/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <filesystem>

#include "engine/assets/AssetManager.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/image/Sampler.hpp"

namespace fgl::engine
{
	class Sampler;
	class Image;
	class BufferSuballocation;
	class ImageView;
	class DescriptorSet;
	class TextureHandle;

	using TextureID = std::uint32_t;

	class Texture;

	using TextureStore = AssetStore< Texture >;

	//TODO: Implement texture handle map to avoid loading the same texture multiple times
	class Texture final : public AssetInterface< Texture >
	{
		template < typename T >
		friend class AssetStore;

		friend class TransferManager;

		//! Key used for the global map keeping track of Textures
		using UIDKeyT = std::filesystem::path;

		//TODO: Implement reusing texture ids
		TextureID m_texture_id;

		std::shared_ptr< ImageView > m_image_view;

		vk::Extent2D m_extent;

		//! Descriptor set used for displaying the texture in ImGui
		vk::DescriptorSet m_imgui_set { VK_NULL_HANDLE };

		[[nodiscard]] Texture( std::tuple< std::vector< std::byte >, int, int, vk::Format > );

		//! Construct texture with a specific extent and data
		[[nodiscard]]
		Texture( std::vector< std::byte >&& data, const int x, const int y, const vk::Format texture_format );

		//! Construct texture with a specific extent and data
		[[nodiscard]]
		Texture( std::vector< std::byte >&& data, const vk::Extent2D extent, const vk::Format texture_format );

		//! Construct with a specific format
		[[nodiscard]] Texture( const std::filesystem::path& path, const vk::Format format );

		//! Construct with no format
		[[nodiscard]] Texture( const std::filesystem::path& path );

	  public:

		inline static UIDKeyT extractKey( const std::filesystem::path& path ) { return path; }

		inline static UIDKeyT extractKey( const std::filesystem::path& path, const vk::Format format ) { return path; }

		Texture() = delete;

		~Texture();

		Texture( const Texture& ) = delete;
		Texture& operator=( const Texture& ) = delete;

		Texture( Texture&& other ) = delete;
		Texture& operator=( Texture&& ) = delete;

		Texture( Image& image, Sampler sampler = Sampler() );

		bool ready() const;

		[[nodiscard]] TextureID getID() const;
		void setName( const std::string& str );

		[[nodiscard]] vk::DescriptorImageInfo getDescriptor() const;
		[[nodiscard]] vk::DescriptorSet& getImGuiDescriptorSet();

		[[nodiscard]] vk::Extent2D getExtent() const;

		[[nodiscard]] ImageView& getImageView();

		void createImGuiSet();

		void drawImGui( vk::Extent2D extent = {} );
		bool drawImGuiButton( vk::Extent2D extent = {} );

		static DescriptorSet& getTextureDescriptorSet();
	};

} // namespace fgl::engine
