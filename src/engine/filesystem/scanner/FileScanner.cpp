//
// Created by kj16609 on 6/5/24.
//

//
// Created by kj16609 on 4/9/23.
//

#include "FileScanner.hpp"

#include "engine/logging/logging.hpp"

namespace fgl::engine::filesystem
{

	DirInfo::DirInfo( const std::filesystem::path& dir, DirInfo* dir_parent ) :
	  path( dir ),
	  total_size( 0 ),
	  parent( dir_parent )
	{
		for ( auto itter = std::filesystem::directory_iterator( dir ); itter != std::filesystem::directory_iterator();
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
	}

	std::size_t DirInfo::fileCount() const
	{
		return files.size();
	}

	FileInfo& DirInfo::file( const std::size_t index )
	{
		return files[ index ];
	}

	DirInfo& DirInfo::dir( const std::size_t index )
	{
		if ( index >= nested_dirs.size() + nested_dirs_to_scan.size() ) throw std::runtime_error( "Index OOB" );

		if ( index >= nested_dirs.size() )
		{
			while ( nested_dirs.size() <= index )
			{
				auto to_scan { std::move( nested_dirs_to_scan.front() ) };
				nested_dirs_to_scan.pop();
				log::debug( "Processed folder: {}", to_scan );

				nested_dirs.emplace_back( to_scan, this );
			}
		}

		return nested_dirs[ index ];
	}

} // namespace fgl::engine::filesystem