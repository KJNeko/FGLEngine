//
// Created by kj16609 on 6/5/24.
//

#pragma once
#include "scanner/FileScanner.hpp"

namespace fgl::engine
{
	struct FrameInfo;
}

namespace fgl::engine::filesystem
{
	class FileScanner;

	struct FileBrowser
	{
		static FileBrowser& getInstance();

		static void addFolderToRoot();

		static void openFolderRoot( const std::string str );

		static void drawGui( FrameInfo& info );
		static void drawFile( const FileInfo& data );
	};

} // namespace fgl::engine::filesystem