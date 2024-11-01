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
	inline static std::unordered_map< std::filesystem::path, std::shared_ptr< Texture > > file_textures {};

	inline static std::unique_ptr< DirInfo > current { nullptr };

	inline static std::once_flag flag {};
	inline static std::shared_ptr< Texture > folder_texture { nullptr };
	inline static std::shared_ptr< Texture > file_texture { nullptr };
	inline static std::shared_ptr< Texture > up_texture { nullptr };
	constexpr std::uint32_t desired_size { 128 };
	constexpr std::uint32_t padding { 2 };

	const std::filesystem::path test_path { std::filesystem::current_path() / "assets" };

	void prepareFileGUI()
	{
		//Prepare textures needed.
		folder_texture = getTextureStore().load( "./assets/folder.png", vk::Format::eR8G8B8A8Unorm );
		file_texture = getTextureStore().load( "./assets/file.png", vk::Format::eR8G8B8A8Unorm );
		up_texture = getTextureStore().load( "./assets/up.png", vk::Format::eR8G8B8A8Unorm );

		current = std::make_unique< DirInfo >( test_path );
	}

	void destroyFileGui()
	{
		folder_texture.reset();
		file_texture.reset();
		up_texture.reset();
		file_textures.clear();
	}

	void FileBrowser::drawGui( [[maybe_unused]] FrameInfo& info )
	{
		ZoneScoped;
		std::call_once( flag, prepareFileGUI );

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

		if ( !current )
		{
			log::critical( "Current has no value!" );
			std::abort();
		}

		auto size { ImGui::GetWindowSize() };
		//TODO: Probably a cleaner way to do this.
		size.x -= 12; // Remove scrollbar pixels

		const float extra { std::fmod( size.x, static_cast< float >( desired_size + ( padding * 2 ) ) ) };
		const auto cols {
			static_cast< int >( ( size.x - extra ) / static_cast< float >( desired_size + ( padding * 2 ) ) )
		};

		if ( cols == 0 )
		{
			log::warn( "Unable to draw cols due to size constraint" );
			return;
		}

		ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, { padding, padding } );

		if ( current && ImGui::BeginTable( "Files", cols ) )
		{
			for ( int i = 0; i < cols; ++i )
				ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, desired_size );

			//List up if we can go up
			if ( current->hasParent() )
			{
				ImGui::TableNextColumn();
				drawUp( current );
			}

			assert( current );

			//List folders first
			for ( std::size_t i = 0; i < current->folderCount(); ++i )
			{
				ImGui::TableNextColumn();
				drawFolder( current->dir( i ) );
			}

			for ( std::size_t i = 0; i < current->fileCount(); ++i )
			{
				ImGui::TableNextColumn();
				drawFile( current->file( i ) );
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

	void drawTexture( const FileInfo& info )
	{
		if ( auto itter = file_textures.find( info.path ); itter != file_textures.end() )
		{
			auto& [ path, texture ] = *itter;

			texture->drawImGuiButton( { desired_size, desired_size } );
		}
		else
		{
			file_texture->drawImGuiButton( { desired_size, desired_size } );

			auto tex { getTextureStore().load( info.path ) };

			// Add the texture
			file_textures.insert( std::make_pair( info.path, std::move( tex ) ) );
		}

		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::SetDragDropPayload(
				DRAG_TYPE_FILE_TEXTURE_INFO, &info, sizeof( info ), ImGuiCond_Once /* Only copy the info once */ );
			ImGui::SetTooltip( info.filename.c_str() );

			ImGui::EndDragDropSource();
		}
	}

	void drawBinary( [[maybe_unused]] const FileInfo& info )
	{
		// file_texture->drawImGui( { 128, 128 } );
		file_texture->drawImGuiButton( { desired_size, desired_size } );

		//Unable to drag/drop because we have no idea what this is supposed to be for.
	}

	void drawModel( const FileInfo& info )
	{
		//TODO: Pre-render preview image for models
		drawBinary( info );

		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::SetDragDropPayload(
				DRAG_TYPE_FILE_MODEL_INFO, &info, sizeof( info ), ImGuiCond_Once /* Only copy the info once */ );
			ImGui::SetTooltip( info.filename.c_str() );

			ImGui::EndDragDropSource();
		}
	}

	void FileBrowser::drawFile( const FileInfo& data )
	{
		ImGui::PushID( data.path.c_str() );

		switch ( data.engine_type )
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

		ImGui::Text( data.filename.c_str() );

		const std::string str { toHumanByteSize( data.size ) };
		ImGui::Text( str.c_str() );
		ImGui::NextColumn();
		ImGui::PopID();
	}

	void FileBrowser::drawFolder( const DirInfo& data )
	{
		ImGui::PushID( data.m_path.c_str() );

		if ( folder_texture->drawImGuiButton( { desired_size, desired_size } ) )
		{
			openFolder( data );
			ImGui::PopID();
			return;
		}

		ImGui::Text( data.m_path.filename().c_str() );
		ImGui::Text( "%ld files\n%ld folders", data.fileCount(), data.folderCount() );

		ImGui::PopID();
	}

	void FileBrowser::goUp()
	{
		current = current->up();
	}

	void FileBrowser::openFolder( const DirInfo& dir )
	{
		file_textures.clear();
		current = std::make_unique< DirInfo >( dir.m_path );
	}

	void FileBrowser::drawUp( const std::unique_ptr< DirInfo >& current_dir )
	{
		const std::unique_ptr< DirInfo > up { current_dir->up() };

		ImGui::PushID( up->m_path.c_str() );

		if ( up_texture->drawImGuiButton( { desired_size, desired_size } ) )
		{
			openFolder( *up );
		}

		ImGui::Text( "Go Up" );

		ImGui::PopID();
	}

} // namespace fgl::engine::filesystem
