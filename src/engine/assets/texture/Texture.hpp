//
// Created by kj16609 on 1/18/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <filesystem>

#include "debug/Track.hpp"
#include "engine/assets/AssetManager.hpp"
#include "engine/assets/image/ImageView.hpp"
#include "engine/assets/image/Sampler.hpp"
#include "engine/constants.hpp"
#include "engine/types.hpp"

namespace fgl::engine
{
	class Sampler;
	class Image;

	namespace memory
	{
		class BufferSuballocation;
	}

	class ImageView;

	namespace descriptors
	{
		class DescriptorSetLayout;
		class DescriptorSet;
	} // namespace descriptors

	class TextureHandle;

	class Texture;

	using TextureStore = AssetStore< Texture >;

	//TODO: Implement texture handle map to avoid loading the same texture multiple times
	class Texture final : public AssetInterface< Texture >
	{
		template < typename T >
		friend class AssetStore;

		friend class TransferManager;

		debug::Track< "GPU", "Texture" > m_gpu_track {};

		//! Key used for the global map keeping track of Textures
		using UIDKeyT = std::filesystem::path;

		//TODO: Implement reusing texture ids
		TextureID m_texture_id;

		std::shared_ptr< Image > m_image;
		std::shared_ptr< ImageView > m_image_view;

		vk::Extent2D m_extent;

		//! Descriptor set used for displaying the texture in ImGui
		vk::DescriptorSet m_imgui_set { VK_NULL_HANDLE };

		std::string m_name;

		[[nodiscard]] Texture( std::tuple< std::vector< std::byte >, int, int, vk::Format, Sampler > );

		//! Construct texture with a specific extent and data
		[[nodiscard]] Texture(
			std::vector< std::byte >&& data, int x, int y, vk::Format texture_format, Sampler&& sampler );

		//! Construct texture with a specific extent and data
		[[nodiscard]] Texture(
			std::vector< std::byte >&& data, vk::Extent2D extent, Sampler&& sampler, vk::Format texture_format );

		//! Construct with a specific format
		[[nodiscard]] Texture( const std::filesystem::path& path, Sampler&&, vk::Format format );

		[[nodiscard]] Texture( const std::filesystem::path& path, Sampler&& sampler );
		[[nodiscard]] Texture( const std::filesystem::path& path );
		[[nodiscard]] Texture( const std::filesystem::path& path, vk::Format format );

	  public:

		static UIDKeyT extractKey( const std::filesystem::path& path, [[maybe_unused]] Sampler&& ) { return path; }

		static UIDKeyT extractKey( const std::filesystem::path& path ) { return path; }

		static UIDKeyT extractKey( const std::filesystem::path& path, [[maybe_unused]] vk::Format ) { return path; }

		Texture() = delete;

		~Texture();
		Texture( const std::shared_ptr< Image >& image, Sampler&& sampler );

		Image& getImageRef();

		Texture( const Texture& ) = delete;
		Texture& operator=( const Texture& ) = delete;

		Texture( Texture&& other ) = delete;
		Texture& operator=( Texture&& ) = delete;

		bool ready() const;

		[[nodiscard]] TextureID getID() const;
		void setName( const std::string& str );

		const std::string& getName() const { return m_name; }

		[[nodiscard]] vk::DescriptorImageInfo getDescriptor() const;
		[[nodiscard]] vk::DescriptorSet& getImGuiDescriptorSet();

		[[nodiscard]] vk::Extent2D getExtent() const;

		[[nodiscard]] ImageView& getImageView();

		void createImGuiSet();

		void drawImGui( vk::Extent2D extent = {} );
		bool drawImGuiButton( vk::Extent2D extent = {} );

		static descriptors::DescriptorSetLayout& getDescriptorLayout();
		static descriptors::DescriptorSet& getDescriptorSet();
	};

} // namespace fgl::engine
