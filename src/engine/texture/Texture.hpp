//
// Created by kj16609 on 1/18/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <filesystem>

#include "TextureHandle.hpp"

namespace fgl::engine
{
	class DescriptorSet;
	class TextureHandle;

	//TODO: Implement texture handle map to avoid loading the same texture multiple times
	class Texture
	{
		std::shared_ptr< TextureHandle > m_handle;

		Texture( std::tuple< std::vector< unsigned char >, int, int, int > );
		Texture( std::shared_ptr< TextureHandle > handle );

	  public:

		Texture( std::vector< unsigned char >& data, const int x, const int y, const int channels );
		Texture( std::vector< unsigned char >& data, const vk::Extent2D extent, const int channels );

		Texture( const Texture& ) = delete;
		Texture& operator=( const Texture& ) = delete;

		Texture( Texture&& other ) = default;
		Texture& operator=( Texture&& ) = default;

		void stage( vk::CommandBuffer& cmd );
		void dropStaging();

		[[nodiscard]] TextureID getID() const;

		[[nodiscard]] vk::DescriptorImageInfo getDescriptor() const;
		[[nodiscard]] vk::DescriptorSet& getImGuiDescriptorSet();

		[[nodiscard]] vk::Extent2D getExtent() const;

		[[nodiscard]] ImageView& getImageView();

		void createImGuiSet();

		static Texture loadFromFile( const std::filesystem::path& path );

		static DescriptorSet& getTextureDescriptorSet();
	};

} // namespace fgl::engine
