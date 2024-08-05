//
// Created by kj16609 on 6/5/24.
//

#pragma once

#include <cassert>
#include <filesystem>
#include <queue>
#include <string>
#include <vector>

namespace fgl::engine::filesystem
{
	struct FileInfo;

	struct DirInfo
	{
		std::filesystem::path m_path;
		std::size_t total_size { 0 };
		std::vector< FileInfo > files {};
		std::vector< DirInfo > nested_dirs {};
		std::queue< std::filesystem::path > nested_dirs_to_scan {};

	  public:

		inline std::unique_ptr< DirInfo > up() const
		{
			assert( std::filesystem::exists( m_path ) );

			return std::make_unique< DirInfo >( m_path.parent_path() );
		}

		std::size_t fileCount() const;
		FileInfo& file( const std::size_t index );

		bool hasParent() const { return !( m_path == "/" || m_path == "" ); }

		inline std::size_t folderCount() const { return nested_dirs.size() + nested_dirs_to_scan.size(); }

		DirInfo& dir( std::size_t index );

		DirInfo() = delete;

		DirInfo( const std::filesystem::path& path );

		DirInfo( const DirInfo& other ) :
		  m_path( other.m_path ),
		  total_size( other.total_size ),
		  files( other.files ),
		  nested_dirs( other.nested_dirs ),
		  nested_dirs_to_scan( other.nested_dirs_to_scan )
		{
			assert( std::filesystem::exists( other.m_path ) );
		}

		DirInfo& operator=( const DirInfo& other )
		{
			assert( std::filesystem::exists( other.m_path ) );
			m_path = other.m_path;
			total_size = other.total_size;
			files = other.files;
			nested_dirs = other.nested_dirs;
			nested_dirs_to_scan = other.nested_dirs_to_scan;
			return *this;
		}

		DirInfo( DirInfo&& ) = default;
		DirInfo& operator=( DirInfo&& ) = default;
	};

	enum EngineFileType
	{
		TEXTURE = 1,
		MODEL = 2,
		SCENE = 3,
		BINARY = 4,
		UNKNOWN,
		DEFAULT = UNKNOWN,
	};

	EngineFileType determineEngineFileType( const std::filesystem::path& path );

	struct FileInfo
	{
		std::string filename;
		std::string ext;
		std::filesystem::path path;
		std::size_t size;
		bool is_folder;
		EngineFileType engine_type;

		FileInfo() = delete;

		FileInfo( const std::filesystem::path& path_in ) :
		  filename( path_in.filename().string() ),
		  ext( path_in.extension().string() ),
		  path( path_in ),
		  size( std::filesystem::file_size( path_in ) ),
		  is_folder( std::filesystem::is_directory( path ) ),
		  engine_type( determineEngineFileType( path_in ) )
		{
			assert( std::filesystem::exists( path_in ) );
		}

		FileInfo( const FileInfo& other ) = default;
		FileInfo& operator=( const FileInfo& other ) = default;

		FileInfo( FileInfo&& other ) = default;
		FileInfo& operator=( FileInfo&& other ) = default;
	};

} // namespace fgl::engine::filesystem
