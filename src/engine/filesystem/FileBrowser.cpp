//
// Created by kj16609 on 6/5/24.
//

#include "FileBrowser.hpp"

#include "engine/assets/stores.hpp"
#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/gui/safe_include.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine::filesystem
{

	inline static std::optional< DirInfo > root {};
	inline static DirInfo* current { nullptr };
	inline static std::once_flag flag {};
	inline static std::shared_ptr< Texture > folder_texture { nullptr };
	inline static std::shared_ptr< Texture > file_texture { nullptr };

	const std::filesystem::path test_path { "/home/kj16609/Desktop/Projects/cxx/Mecha/models" };

	void prepareFileGUI()
	{
		ZoneScoped;

		//Prepare textures needed.
		folder_texture = getTextureStore().load( "./models/folder.png", vk::Format::eR8G8B8A8Unorm );
		file_texture = getTextureStore().load( "./models/file.png", vk::Format::eR8G8B8A8Unorm );

		auto cmd_buffer { Device::getInstance().beginSingleTimeCommands() };

		Device::getInstance().endSingleTimeCommands( cmd_buffer );

		root = DirInfo( test_path );
		current = &root.value();
	}

	void FileBrowser::drawGui( FrameInfo& info )
	{
		ZoneScoped;
		//std::call_once( flag, []() { scanners.emplace_back( std::make_unique< FileScanner >( test_path ) ); } );
		std::call_once( flag, []() { prepareFileGUI(); } );

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

		const auto size { ImGui::GetWindowSize() };
		constexpr float desired_size { 128.0f };
		const float extra { std::fmod( size.x, desired_size ) };
		const auto cols { ( size.x - extra ) / desired_size };

		if ( current && ImGui::BeginTable( "Files", cols ) )
		{
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

	void FileBrowser::drawFile( FileInfo& data )
	{
		ZoneScoped;
		ImGui::PushID( data.path.c_str() );
		ImGui::Text( data.filename.c_str() );
		ImGui::Text( data.ext.c_str() );

		// file_texture->drawImGui( { 128, 128 } );
		file_texture->drawImGuiButton( { 128, 128 } );
		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::
				SetDragDropPayload( "_FILE_INFO", &data, sizeof( data ), ImGuiCond_Once /* Only copy the data once */ );
			ImGui::SetTooltip( data.filename.c_str() );

			ImGui::EndDragDropSource();
		}

		ImGui::SameLine();
		ImGui::Text( "%0.1f KB", static_cast< float >( data.size ) / 1024.0f );
		ImGui::NextColumn();
		ImGui::PopID();
	}

	void FileBrowser::drawFolder( DirInfo& data )
	{
		ZoneScoped;
		ImGui::PushID( data.path.c_str() );

		ImGui::Text( data.path.filename().c_str() );
		ImGui::Text( "Folder" );

		if ( folder_texture->drawImGuiButton( { 128, 128 } ) )
		{
			openFolder( data );
		}

		ImGui::PopID();
	}

	void FileBrowser::openFolder( DirInfo& dir )
	{
		if ( !current ) throw std::runtime_error( "No current folder?" );

		current = &dir;
	}

} // namespace fgl::engine::filesystem
