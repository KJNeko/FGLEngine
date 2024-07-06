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
		static void goUp();

		static void openFolder( const DirInfo& dir );

		static void drawUp( const std::unique_ptr< DirInfo >& up );
		static void drawGui( FrameInfo& info );
		static void drawFile( const FileInfo& data );
		static void drawFolder( const DirInfo& data );
	};

} // namespace fgl::engine::filesystem