//
// Created by kj16609 on 6/5/24.
//

#pragma once

#include <coroutine>
#include <filesystem>
#include <queue>
#include <string>
#include <vector>

namespace fgl::engine::filesystem
{
	struct FileInfo;

	struct DirInfo
	{
		std::filesystem::path path;
		std::size_t total_size;
		std::vector< FileInfo > files {};
		std::vector< DirInfo > nested_dirs {};
		std::queue< std::filesystem::path > nested_dirs_to_scan {};
		DirInfo* parent { nullptr };

	  public:

		inline DirInfo* up() const { return parent; }

		std::size_t fileCount() const;
		FileInfo& file( const std::size_t index );

		inline std::size_t folderCount() const { return nested_dirs.size() + nested_dirs_to_scan.size(); }

		DirInfo& dir( const std::size_t index );

		DirInfo() = delete;

		DirInfo( const std::filesystem::path& path, DirInfo* parent = nullptr );

		DirInfo( const DirInfo& other ) = delete;
		DirInfo& operator=( const DirInfo& other ) = delete;

		DirInfo( DirInfo&& ) = default;
		DirInfo& operator=( DirInfo&& ) = default;
	};

	struct FileInfo
	{
		std::string filename;
		std::string ext;
		std::filesystem::path path;
		std::size_t size;
		bool is_folder;

		FileInfo() = delete;

		FileInfo( std::filesystem::path path_in ) :
		  filename( path_in.filename().string() ),
		  ext( path_in.extension().string() ),
		  path( path_in ),
		  size( std::filesystem::file_size( path_in ) ),
		  is_folder( std::filesystem::is_directory( path ) )
		{}

		FileInfo( const FileInfo& other ) = delete;
		FileInfo& operator=( const FileInfo& other ) = delete;

		FileInfo( FileInfo&& other ) = default;
		FileInfo& operator=( FileInfo&& other ) = default;
	};

} // namespace fgl::engine::filesystem
