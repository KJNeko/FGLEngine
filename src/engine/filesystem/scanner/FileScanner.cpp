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

	DirInfo::DirInfo( const std::filesystem::path& path ) : m_path( path ), m_total_size( 0 )
	{
		FGL_ASSERT( std::filesystem::exists( path ), "Path must exist" );
		for ( auto itter = std::filesystem::directory_iterator( path ); itter != std::filesystem::directory_iterator();
		      ++itter )
		{
			if ( itter->is_regular_file() )
			{
				m_files.emplace_back( *itter );
			}
			else if ( itter->is_directory() )
			{
				m_nested_dirs_to_scan.push( *itter );
			}
			else
			{
				log::debug( "Weird file at {}", itter->path().string() );
			}
		}

		m_nested_dirs.reserve( m_nested_dirs_to_scan.size() );
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
		return m_files.size();
	}

	FileInfo& DirInfo::file( const std::size_t index )
	{
		return m_files[ index ];
	}

	DirInfo& DirInfo::dir( std::size_t index )
	{
		if ( index >= m_nested_dirs.size() + m_nested_dirs_to_scan.size() ) throw std::runtime_error( "Index OOB" );

		if ( index >= m_nested_dirs.size() )
		{
			while ( m_nested_dirs.size() <= index )
			{
				std::filesystem::path to_scan { std::move( m_nested_dirs_to_scan.front() ) };
				m_nested_dirs_to_scan.pop();

				DirInfo info { to_scan };

				m_nested_dirs.push_back( std::move( info ) );
			}
		}

		return m_nested_dirs[ index ];
	}

} // namespace fgl::engine::filesystem