//
// Created by kj16609 on 6/5/24.
//

//
// Created by kj16609 on 4/9/23.
//

#include "FileScanner.hpp"

#include <unordered_map>

#include "engine/FGL_DEFINES.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::filesystem
{

	DirInfo::DirInfo( const std::filesystem::path& path ) : m_path( path ), total_size( 0 )
	{
		FGL_ASSERT( std::filesystem::exists( path ), "Path must exist" );
		for ( auto itter = std::filesystem::directory_iterator( path ); itter != std::filesystem::directory_iterator();
		      ++itter )
		{
			if ( itter->is_regular_file() )
			{
				files.emplace_back( *itter );
			}
			else if ( itter->is_directory() )
			{
				nested_dirs_to_scan.push( *itter );
			}
			else
				throw std::runtime_error( "Unknown/Unspported file type" );
		}

		nested_dirs.reserve( nested_dirs_to_scan.size() );
	}

	EngineFileType determineEngineFileType( const std::filesystem::path& path )
	{
		const auto& ext { path.extension() };

		//TODO: Never trust file extensions
		static const std::unordered_map< EngineFileType, std::vector< std::string_view > > extension_map {
			{ EngineFileType::MODEL, { ".obj" } },
			{ EngineFileType::TEXTURE, { ".png", ".jpg" } },
			{ EngineFileType::SCENE, { ".glb", ".gltf" } }
		};

		for ( const auto& [ type, strings ] : extension_map )
		{
			for ( const auto& str : strings )
			{
				if ( str == ext ) return type;
			}
		}

		return EngineFileType::UNKNOWN;
	}

	std::size_t DirInfo::fileCount() const
	{
		return files.size();
	}

	FileInfo& DirInfo::file( const std::size_t index )
	{
		return files[ index ];
	}

	DirInfo& DirInfo::dir( std::size_t index )
	{
		if ( index >= nested_dirs.size() + nested_dirs_to_scan.size() ) throw std::runtime_error( "Index OOB" );

		if ( index >= nested_dirs.size() )
		{
			while ( nested_dirs.size() <= index )
			{
				std::filesystem::path to_scan { std::move( nested_dirs_to_scan.front() ) };
				nested_dirs_to_scan.pop();
				log::debug( "Processed folder: {}", to_scan );

				DirInfo info { to_scan };

				nested_dirs.push_back( std::move( info ) );
			}
		}

		return nested_dirs[ index ];
	}

} // namespace fgl::engine::filesystem