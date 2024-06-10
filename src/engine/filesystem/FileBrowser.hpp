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

		static void goUp();

		static void addFolderToRoot();

		static void openFolder( DirInfo& dir );

		static void drawGui( FrameInfo& info );
		static void drawFile( FileInfo& data );
		static void drawFolder( DirInfo& data );
	};

} // namespace fgl::engine::filesystem