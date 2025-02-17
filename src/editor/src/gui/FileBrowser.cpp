//
// Created by kj16609 on 6/5/24.
//

#include "FileBrowser.hpp"

#include <cmath>

#include "engine/assets/image/ImageView.hpp"
#include "engine/assets/stores.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/filesystem/types.hpp"
#include "safe_include.hpp"

namespace fgl::engine::filesystem
{

	//! Textures for files (pre-rendered image, images, ect)

	constexpr std::uint32_t DESIRED_SIZE { 128 };
	constexpr std::uint32_t PADDING { 2 };

	const std::filesystem::path TEST_PATH { std::filesystem::current_path() / "assets" };

	void FileBrowser::drawGui( [[maybe_unused]] FrameInfo& info )
	{
		ZoneScoped;

		/*
		if ( ImGui::BeginMenuBar() )
		{
			if ( ImGui::BeginMenu( "File" ) )
			{
				if ( ImGui::MenuItem( "Open File" ) )
				{}

				if ( ImGui::MenuItem( "Open Folder" ) )
				{}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		*/

		if ( !m_current_dir )
		{
			m_current_dir = std::make_unique< DirInfo >( std::filesystem::current_path() );
			log::warn(
				"Current directory was not set, Defaulting to current directory: {}", std::filesystem::current_path() );
		}

		auto size { ImGui::GetWindowSize() };
		//TODO: Probably a cleaner way to do this.
		size.x -= 12; // Remove scrollbar pixels

		const float extra { std::fmod( size.x, static_cast< float >( DESIRED_SIZE + ( PADDING * 2 ) ) ) };
		const auto cols {
			static_cast< int >( ( size.x - extra ) / static_cast< float >( DESIRED_SIZE + ( PADDING * 2 ) ) )
		};

		if ( cols == 0 )
		{
			log::warn( "Unable to draw cols due to size constraint" );
			return;
		}

		ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, { PADDING, PADDING } );

		if ( m_current_dir && ImGui::BeginTable( "Files", cols ) )
		{
			for ( int i = 0; i < cols; ++i )
				ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, DESIRED_SIZE );

			//List up if we can go up
			if ( m_current_dir->hasParent() )
			{
				ImGui::TableNextColumn();
				drawUp( m_current_dir );
			}

			assert( m_current_dir );

			//List folders first
			for ( std::size_t i = 0; i < m_current_dir->folderCount(); ++i )
			{
				ImGui::TableNextColumn();
				drawFolder( m_current_dir->dir( i ) );
			}

			for ( std::size_t i = 0; i < m_current_dir->fileCount(); ++i )
			{
				ImGui::TableNextColumn();
				drawFile( m_current_dir->file( i ) );
			}

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();

		ImGui::Columns( 1 );
	}

	std::string toHumanByteSize( size_t size )
	{
		if ( size < 1000 )
		{
			return format_ns::format( "{} B", size );
		}
		else if ( size < 1000 * 1000 )
		{
			return format_ns::format( "{:0.2f} KB", static_cast< float >( size ) / 1000.0f );
		}
		else if ( size < 1000 * 1000 * 1000 )
		{
			return format_ns::format( "{:0.2f} MB", static_cast< float >( size ) / 1000.0f / 1000.0f );
		}

		return format_ns::format( "{:0.2f} GB", static_cast< float >( size ) / 1000.0f / 1000.0f / 1000.0f );
	}

	void FileBrowser::drawTexture( const FileInfo& info )
	{
		if ( auto itter = m_file_textures.find( info.m_path ); itter != m_file_textures.end() )
		{
			auto& [ path, texture ] = *itter;

			texture->drawImGuiButton( { DESIRED_SIZE, DESIRED_SIZE } );
		}
		else
		{
			m_file_texture->drawImGuiButton( { DESIRED_SIZE, DESIRED_SIZE } );

			auto tex { getTextureStore().load( info.m_path ) };

			// Add the texture
			m_file_textures.insert( std::make_pair( info.m_path, std::move( tex ) ) );
		}

		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::SetDragDropPayload(
				DRAG_TYPE_FILE_TEXTURE_INFO, &info, sizeof( info ), ImGuiCond_Once /* Only copy the info once */ );
			ImGui::SetTooltip( info.m_filename.c_str() );

			ImGui::EndDragDropSource();
		}
	}

	void FileBrowser::drawBinary( [[maybe_unused]] const FileInfo& info )
	{
		// file_texture->drawImGui( { 128, 128 } );
		m_file_texture->drawImGuiButton( { DESIRED_SIZE, DESIRED_SIZE } );

		//Unable to drag/drop because we have no idea what this is supposed to be for.
	}

	void FileBrowser::drawModel( const FileInfo& info )
	{
		//TODO: Pre-render preview image for models
		drawBinary( info );

		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::SetDragDropPayload(
				DRAG_TYPE_FILE_MODEL_INFO, &info, sizeof( info ), ImGuiCond_Once /* Only copy the info once */ );
			ImGui::SetTooltip( info.m_filename.c_str() );

			ImGui::EndDragDropSource();
		}
	}

	void FileBrowser::drawFile( const FileInfo& data )
	{
		ImGui::PushID( data.m_path.c_str() );

		switch ( data.m_engine_type )
		{
			case TEXTURE:
				drawTexture( data );
				break;
			case MODEL:
				[[fallthrough]];
			case SCENE:
				drawModel( data );
				break;
			default:
				[[fallthrough]];
			case BINARY:
				[[fallthrough]];
			case UNKNOWN:
				drawBinary( data );
				break;
		}

		ImGui::Text( data.m_filename.c_str() );

		const std::string str { toHumanByteSize( data.m_size ) };
		ImGui::Text( str.c_str() );
		ImGui::NextColumn();
		ImGui::PopID();
	}

	void FileBrowser::drawFolder( const DirInfo& data )
	{
		ImGui::PushID( data.m_path.c_str() );

		if ( m_folder_texture->drawImGuiButton( { DESIRED_SIZE, DESIRED_SIZE } ) )
		{
			openFolder( data );
			ImGui::PopID();
			return;
		}

		ImGui::Text( data.m_path.filename().c_str() );
		ImGui::Text( "%ld files\n%ld folders", data.fileCount(), data.folderCount() );

		ImGui::PopID();
	}

	FileBrowser::FileBrowser() : m_current_dir( std::make_unique< DirInfo >( TEST_PATH ) )
	{
		m_folder_texture = getTextureStore().load( "./assets/folder.png", vk::Format::eR8G8B8A8Unorm );
		m_file_texture = getTextureStore().load( "./assets/file.png", vk::Format::eR8G8B8A8Unorm );
		m_up_texture = getTextureStore().load( "./assets/up.png", vk::Format::eR8G8B8A8Unorm );
	}

	void FileBrowser::goUp()
	{
		m_current_dir = m_current_dir->up();
	}

	void FileBrowser::openFolder( const DirInfo& dir )
	{
		m_file_textures.clear();
		m_current_dir = std::make_unique< DirInfo >( dir.m_path );
	}

	void FileBrowser::drawUp( const std::unique_ptr< DirInfo >& current_dir )
	{
		const std::unique_ptr< DirInfo > up { current_dir->up() };

		ImGui::PushID( up->m_path.c_str() );

		if ( m_up_texture->drawImGuiButton( { DESIRED_SIZE, DESIRED_SIZE } ) )
		{
			openFolder( *up );
		}

		ImGui::Text( "Go Up" );

		ImGui::PopID();
	}

} // namespace fgl::engine::filesystem
