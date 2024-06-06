//
// Created by kj16609 on 6/5/24.
//

//
// Created by kj16609 on 4/9/23.
//

#include "FileScanner.hpp"

#include <queue>

#include "engine/logging/logging.hpp"

namespace fgl::engine::filesystem
{

	FileInfo& FileScannerGenerator::operator()()
	{
		if ( m_h.done() ) throw std::runtime_error( "FileScannerGenerator is done but operator was still called" );
		m_h();

		if ( m_h.promise().exception ) std::rethrow_exception( m_h.promise().exception );
		if ( m_h.promise().value.has_value() )
		{
			return m_h.promise().value.value();
		}
		else
			throw std::runtime_error( "Failed to get value from FileScannerGenerator." );
	}

	FileScannerGenerator scan_files( const std::filesystem::path path )
	{
		if ( !std::filesystem::exists( path ) )
		{
			log::error( "Expected path does not exist: {}", path.string() );
			throw std::runtime_error( format_ns::format( "Path {} does not exist.", path ).c_str() );
		}

		auto dir_empty = []( const std::filesystem::path& dir_path ) -> bool
		{ return std::filesystem::directory_iterator( dir_path ) == std::filesystem::directory_iterator(); };

		if ( dir_empty( path ) ) co_return FileInfo { path, path, 0, 0 };

		std::queue< std::pair< std::filesystem::path, std::uint8_t > > dirs {};

		dirs.push( { path, 0 } );

		while ( dirs.size() > 0 )
		{
			const auto [ dir, depth ] { std::move( dirs.front() ) };
			dirs.pop();
			std::vector< std::filesystem::path > nested_dirs {};

			log::debug( "Searching {}", dir );

			//Recurse through the directory.
			for ( auto itter = std::filesystem::directory_iterator( dir );
			      itter != std::filesystem::directory_iterator(); )
			{
				log::debug( "Found: {}", dir );
				if ( itter->is_directory() )
				{
					//Add directory to scan list.
					nested_dirs.emplace_back( *itter );
					++itter;
					continue;
				}

				FileInfo info {
					*itter, path, itter->is_regular_file() ? itter->file_size() : 0, std::uint8_t( depth + 1 )
				};

				++itter;

				//If we are at the last file and there are no more directories to scan then return.
				if ( itter == std::filesystem::directory_iterator() && dirs.size() == 0 && nested_dirs.size() == 0 )
				{
					log::debug( "co_return: {}", info.path );
					co_return std::move( info );
				}
				else
				{
					log::debug( "co_yield: {}", info.path );
					co_yield std::move( info );
				}
			}

			//Add the nested dirs to the scanlist and yield them
			for ( std::size_t i = 0; i < nested_dirs.size(); ++i )
			{
				FileInfo info { nested_dirs.at( i ), path, 0, std::uint8_t( depth + 1 ) };
				// Check if the directory is empty and if it's not then add it to the scan queue.

				const bool is_empty { dir_empty( nested_dirs.at( i ) ) };

				//If the dir is empty, if we are done searching our current nested list, and there are no more dirs to process, Return
				if ( is_empty && i == nested_dirs.size() - 1 && dirs.size() == 0 )
				{
					log::debug( "co_return: {}", info.path );
					co_return std::move( info );
				}

				if ( !is_empty )
				{
					dirs.push( { nested_dirs.at( i ), depth + 1 } );
				}

				co_yield std::move( info );
			}
		}

		spdlog::critical( "Got to an illegal spot!" );
		std::unreachable();
	}

	FileScanner::FileScanner( const std::filesystem::path& path ) : m_path( path ), file_scanner( scan_files( path ) )
	{
		files.emplace_back( std::move( file_scanner() ) );
	}

	const FileInfo& FileScanner::at( std::size_t index )
	{
		if ( index >= files.size() && !file_scanner.m_h.done() )
		{
			// Index is higher then what we have.
			// Scanner is also NOT done.
			// We use the coroutine to fetch what the next file should be.

			std::size_t diff { index - ( files.size() - 1 ) };

			while ( diff > 0 && !file_scanner.m_h.done() )
			{
				files.emplace_back( std::move( file_scanner() ) );
				--diff;
			}
		}

		if ( index >= files.size() )
			throw std::
				runtime_error( format_ns::format( "index = {}: size < index : {} < {}", index, files.size(), index )
			                       .c_str() );

		return files.at( index );
	}

	bool FileScanner::iterator::operator==( const std::unreachable_sentinel_t ) const
	{
		return m_scanner.file_scanner.m_h.done() && ( m_idx == m_scanner.files.size() );
	}
} // namespace fgl::engine::filesystem