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
		std::size_t m_total_size { 0 };
		std::vector< FileInfo > m_files {};
		std::vector< DirInfo > m_nested_dirs {};
		std::queue< std::filesystem::path > m_nested_dirs_to_scan {};

	  public:

		std::unique_ptr< DirInfo > up() const
		{
			assert( std::filesystem::exists( m_path ) );

			return std::make_unique< DirInfo >( m_path.parent_path() );
		}

		std::size_t fileCount() const;
		FileInfo& file( const std::size_t index );

		bool hasParent() const { return !( m_path == "/" || m_path == "" ); }

		std::size_t folderCount() const { return m_nested_dirs.size() + m_nested_dirs_to_scan.size(); }

		DirInfo& dir( std::size_t index );

		DirInfo() = delete;

		DirInfo( const std::filesystem::path& path );

		DirInfo( const DirInfo& other ) :
		  m_path( other.m_path ),
		  m_total_size( other.m_total_size ),
		  m_files( other.m_files ),
		  m_nested_dirs( other.m_nested_dirs ),
		  m_nested_dirs_to_scan( other.m_nested_dirs_to_scan )
		{
			assert( std::filesystem::exists( other.m_path ) );
		}

		DirInfo& operator=( const DirInfo& other )
		{
			assert( std::filesystem::exists( other.m_path ) );
			m_path = other.m_path;
			m_total_size = other.m_total_size;
			m_files = other.m_files;
			m_nested_dirs = other.m_nested_dirs;
			m_nested_dirs_to_scan = other.m_nested_dirs_to_scan;
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
		std::string m_filename;
		std::string m_ext;
		std::filesystem::path m_path;
		std::size_t m_size;
		bool m_is_folder;
		EngineFileType m_engine_type;

		FileInfo() = delete;

		explicit FileInfo( const std::filesystem::path& path_in ) :
		  m_filename( path_in.filename().string() ),
		  m_ext( path_in.extension().string() ),
		  m_path( path_in ),
		  m_size( std::filesystem::file_size( path_in ) ),
		  m_is_folder( std::filesystem::is_directory( m_path ) ),
		  m_engine_type( determineEngineFileType( path_in ) )
		{
			assert( std::filesystem::exists( path_in ) );
		}

		FileInfo( const FileInfo& other ) = default;
		FileInfo& operator=( const FileInfo& other ) = default;

		FileInfo( FileInfo&& other ) = default;
		FileInfo& operator=( FileInfo&& other ) = default;
	};

} // namespace fgl::engine::filesystem
