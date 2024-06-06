//
// Created by kj16609 on 6/5/24.
//

#pragma once
#ifndef ATLASGAMEMANAGER_FILESCANNER_HPP
#define ATLASGAMEMANAGER_FILESCANNER_HPP

#include <coroutine>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

#include "engine/logging/logging.hpp"

namespace fgl::engine::filesystem
{

	struct FileInfo
	{
		std::string filename;
		std::string ext;
		std::filesystem::path path;
		std::size_t size;
		std::uint8_t depth;
		std::filesystem::path relative;
		bool is_folder;

		FileInfo() = delete;

		FileInfo(
			std::filesystem::path path_in,
			const std::filesystem::path& source,
			const std::size_t filesize,
			const std::uint8_t file_depth ) :
		  filename( path_in.filename().string() ),
		  ext( path_in.extension().string() ),
		  path( path_in ),
		  size( filesize ),
		  depth( file_depth ),
		  relative( std::filesystem::relative( std::move( path_in ), source ) ),
		  is_folder( std::filesystem::is_directory( path ) )
		{}

		FileInfo( const FileInfo& other ) = delete;
		FileInfo& operator=( const FileInfo& other ) = delete;

		FileInfo( FileInfo&& other ) = default;
		FileInfo& operator=( FileInfo&& other ) = default;
	};

	struct FileScannerGenerator
	{
		struct promise_type;
		using handle_type = std::coroutine_handle< promise_type >;

		struct promise_type
		{
			std::optional< FileInfo > value { std::nullopt };
			std::exception_ptr exception { nullptr };

			FileScannerGenerator get_return_object()
			{
				return FileScannerGenerator( handle_type::from_promise( *this ) );
			}

			std::suspend_always initial_suspend() noexcept { return {}; }

			std::suspend_always final_suspend() noexcept { return {}; }

			void unhandled_exception()
			{
				exception = std::current_exception();
				log::critical( "Exception thrown in FileScanner" );
			}

			void return_value( FileInfo&& from )
			{
				if ( from.filename == "" )
					throw std::runtime_error( "FileScannerGenerator: return value had no filename!" );

				value = std::move( from );
			}

			std::suspend_always yield_value( FileInfo&& from )
			{
				if ( from.filename == "" )
					throw std::runtime_error( "FromScannerGenerator:: yield value had no filename!" );

				value = std::forward< FileInfo >( from );
				return {};
			}
		};

		handle_type m_h;

		FileScannerGenerator() = delete;

		FileScannerGenerator( handle_type h ) : m_h( h ) {}

		~FileScannerGenerator()
		{
			log::debug( "Destroying Generator" );
			m_h.destroy();
		}

		FileInfo& operator()();
	};

	class FileScanner
	{
	  private:

		std::filesystem::path m_path;
		FileScannerGenerator file_scanner;
		std::vector< FileInfo > files {};
		const FileInfo& at( std::size_t index );

		friend class iterator;

		class iterator
		{
			std::size_t m_idx { 0 };
			FileScanner& m_scanner;

		  public:

			iterator() = delete;

			iterator( const std::size_t idx, FileScanner& scanner ) : m_idx( idx ), m_scanner( scanner ) {}

			FileScanner::iterator& operator++()
			{
				++m_idx;
				return *this;
			}

			// Operator != required to check for end I assume. Where if the this returns true then we are good to continue
			// So instead we can just return the state of the scanner. And if the scanner is complete then we'll return false here.
			//bool operator !=
			bool operator==( const std::unreachable_sentinel_t ) const;

			// Required for the for loop
			const FileInfo& operator*() { return m_scanner.at( m_idx ); }
		};

	  public:

		FileScanner() = delete;
		FileScanner( const std::filesystem::path& path );

		FileScanner( const std::string_view path ) : FileScanner( std::filesystem::path( path ) ) {}

		FileScanner( const FileScanner& other ) = delete; //Copy
		FileScanner& operator=( const FileScanner& other ) = delete; //Copy-Assign

		iterator begin() { return iterator( 0, *this ); }

		//This *probably* isn't required(?) but the for loop will want it anyways. So we can just return literaly anything here since it's not used anyways.
		std::unreachable_sentinel_t end() { return {}; }

		std::filesystem::path path() const { return m_path; }
	};
} // namespace fgl::engine::filesystem
#endif //ATLASGAMEMANAGER_FILESCANNER_HPP
