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
		//! Has this texture been submitted to the GPU?
		bool submitte_to_gpu { false };

		[[nodiscard]] Texture( const std::tuple< std::vector< std::byte >, int, int, int >& );
		[[nodiscard]] Texture( std::shared_ptr< TextureHandle > handle );

	  public:

		[[nodiscard]] Texture( const std::vector< std::byte >& data, const int x, const int y, const int channels );
		[[nodiscard]] Texture( const std::vector< std::byte >& data, const vk::Extent2D extent, const int channels );

		Texture( const Texture& ) = default;
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

		[[nodiscard]] static Texture generateFromPerlinNoise( int x_size, int y_size, std::size_t seed = 0 );
		[[nodiscard]] static Texture loadFromFile( const std::filesystem::path& path );

		static DescriptorSet& getTextureDescriptorSet();
	};

} // namespace fgl::engine
