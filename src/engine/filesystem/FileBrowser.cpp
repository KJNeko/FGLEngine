//
// Created by kj16609 on 6/5/24.
//

#include "FileBrowser.hpp"

#include "engine/assets/stores.hpp"
#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/gui/safe_include.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/logging/logging.hpp"
#include "engine/texture/Texture.hpp"
#include "types.hpp"

namespace fgl::engine::filesystem
{

	//! Textures for files (pre-rendered image, images, ect)
	inline static std::unordered_map< std::filesystem::path, Texture > file_textures {};
	inline static std::optional< DirInfo > current { std::nullopt };
	inline static std::once_flag flag {};
	inline static std::shared_ptr< Texture > folder_texture { nullptr };
	inline static std::shared_ptr< Texture > file_texture { nullptr };
	inline static std::shared_ptr< Texture > up_texture { nullptr };
	constexpr std::uint32_t desired_size { 128 };
	constexpr std::uint32_t padding { 2 };

	const std::filesystem::path test_path { "/home/kj16609/Desktop/Projects/cxx/Mecha/assets" };

	void prepareFileGUI()
	{
		ZoneScoped;

		//Prepare textures needed.
		folder_texture = getTextureStore().load( "./assets/folder.png", vk::Format::eR8G8B8A8Unorm );
		file_texture = getTextureStore().load( "./assets/file.png", vk::Format::eR8G8B8A8Unorm );
		up_texture = getTextureStore().load( "./assets/up.png", vk::Format::eR8G8B8A8Unorm );

		current = DirInfo( test_path );
	}

	void FileBrowser::drawGui( FrameInfo& info )
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

		if ( !current.has_value() )
		{
			log::warn( "Current has no value!" );
		}

		auto size { ImGui::GetWindowSize() };
		size.x -= 12; // Remove scrollbar pixels

		const float extra { std::fmod( size.x, desired_size + ( padding * 2 ) ) };
		const auto cols { ( size.x - extra ) / static_cast< float >( desired_size + ( padding * 2 ) ) };

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
				drawUp( *current );
			}

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

	enum FileType
	{
		TEXTURE,
		MODEL,
		BINARY,
		DEFAULT = BINARY,
	};

	FileType getFileType( const std::filesystem::path path )
	{
		//TODO: NEVER TRUST FILE EXTENSIONS!
		const auto extension { path.extension() };

		//Map
		static const std::map< FileType, std::vector< std::string_view > > map {
			{ TEXTURE, { ".jpg", ".png" } }, { MODEL, { ".glb", ".obj", ".gltf" } }
		};

		for ( const auto& [ type, extensions ] : map )
		{
			//Check if the file extensions matches the list for this type
			if ( std::find( extensions.begin(), extensions.end(), extension ) != extensions.end() )
			{
				return type;
			}
		}

		//Default
		return DEFAULT;
	}

	void drawTexture()
	{}

	void drawModel()
	{}

	void drawBinary()
	{}

	void FileBrowser::drawFile( const FileInfo& data )
	{
		ZoneScoped;
		ImGui::PushID( data.path.c_str() );

		// file_texture->drawImGui( { 128, 128 } );
		file_texture->drawImGuiButton( { desired_size, desired_size } );
		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::SetDragDropPayload(
				DRAG_TYPE_FILE_INFO, &data, sizeof( data ), ImGuiCond_Once /* Only copy the data once */ );
			ImGui::SetTooltip( data.filename.c_str() );

			ImGui::EndDragDropSource();
		}

		ImGui::Text( data.filename.c_str() );
		ImGui::Text( data.ext.c_str() );

		ImGui::SameLine();
		ImGui::Text( "%0.1f KB", static_cast< float >( data.size ) / 1024.0f );
		ImGui::NextColumn();
		ImGui::PopID();
	}

	void FileBrowser::drawFolder( const DirInfo& data )
	{
		ZoneScoped;
		ImGui::PushID( data.path.c_str() );

		if ( folder_texture->drawImGuiButton( { desired_size, desired_size } ) )
		{
			openFolder( data );
		}

		ImGui::Text( data.path.filename().c_str() );
		ImGui::Text( "%ld files\n%ld folders", data.fileCount(), data.folderCount() );

		ImGui::PopID();
	}

	void FileBrowser::goUp()
	{
		current = current->up();
	}

	void FileBrowser::openFolder( DirInfo dir )
	{
		current = dir;
	}

	void FileBrowser::drawUp( const DirInfo& data )
	{
		ImGui::PushID( data.path.c_str() );

		if ( up_texture->drawImGuiButton( { desired_size, desired_size } ) )
		{
			openFolder( data.up() );
		}

		ImGui::Text( "Go Up" );

		ImGui::PopID();
	}

} // namespace fgl::engine::filesystem
