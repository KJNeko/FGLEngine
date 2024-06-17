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
		std::size_t total_size { 0 };
		std::vector< FileInfo > files {};
		std::vector< DirInfo > nested_dirs {};
		std::queue< std::filesystem::path > nested_dirs_to_scan {};

	  public:

		inline DirInfo up() const
		{
			assert( std::filesystem::exists( path ) );
			return DirInfo( path.parent_path() );
		}

		std::size_t fileCount() const;
		FileInfo& file( const std::size_t index );

		bool hasParent() const { return !( path == "/" || path == "" ); }

		inline std::size_t folderCount() const { return nested_dirs.size() + nested_dirs_to_scan.size(); }

		DirInfo& dir( std::size_t index );

		DirInfo() = delete;

		DirInfo( const std::filesystem::path& path );

		DirInfo( const DirInfo& other ) :
		  path( other.path ),
		  total_size( other.total_size ),
		  files( other.files ),
		  nested_dirs( other.nested_dirs ),
		  nested_dirs_to_scan( other.nested_dirs_to_scan )
		{
			assert( std::filesystem::exists( other.path ) );
		}

		DirInfo& operator=( const DirInfo& other )
		{
			assert( std::filesystem::exists( other.path ) );
			path = other.path;
			total_size = other.total_size;
			files = other.files;
			nested_dirs = other.nested_dirs;
			nested_dirs_to_scan = other.nested_dirs_to_scan;
			return *this;
		}

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
		{
			assert( std::filesystem::exists( path_in ) );
		}

		FileInfo( const FileInfo& other ) = default;
		FileInfo& operator=( const FileInfo& other ) = default;

		FileInfo( FileInfo&& other ) = default;
		FileInfo& operator=( FileInfo&& other ) = default;
	};

} // namespace fgl::engine::filesystem
