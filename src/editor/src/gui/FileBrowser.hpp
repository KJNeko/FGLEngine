//
// Created by kj16609 on 6/5/24.
//

#pragma once
#include <mutex>
#include <unordered_map>

#include "../../../engine/filesystem/scanner/FileScanner.hpp"
#include "assets/texture/Texture.hpp"

namespace fgl::engine
{
	struct FrameInfo;
}

namespace fgl::engine::filesystem
{
	class FileScanner;

	struct FileBrowser
	{
		std::unordered_map< std::filesystem::path, std::shared_ptr< Texture > > m_file_textures {};

		std::unique_ptr< DirInfo > m_current_dir { nullptr };

		std::shared_ptr< Texture > m_folder_texture { nullptr };
		std::shared_ptr< Texture > m_file_texture { nullptr };
		std::shared_ptr< Texture > m_up_texture { nullptr };

		FileBrowser();

		void goUp();

		void openFolder( const DirInfo& dir );

		void drawUp( const std::unique_ptr< DirInfo >& current_dir );
		void drawGui( FrameInfo& info );
		void drawFile( const FileInfo& data );
		void drawFolder( const DirInfo& data );

		// drawers
		void drawBinary( const FileInfo& info );
		void drawModel( const FileInfo& info );
		void drawTexture( const FileInfo& info );
	};

	void destroyFileGui();

} // namespace fgl::engine::filesystem