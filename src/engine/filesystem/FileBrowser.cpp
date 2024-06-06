//
// Created by kj16609 on 6/5/24.
//

#include "FileBrowser.hpp"

#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/gui/safe_include.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/image/Sampler.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine::filesystem
{

	inline static std::vector< std::unique_ptr< FileScanner > > scanners {};
	inline static FileScanner* current_scanner { nullptr };
	inline static std::once_flag flag {};
	inline static std::optional< Texture > folder_texture { std::nullopt };

	const std::filesystem::path path { "/home/kj16609/Desktop/Projects/cxx/Mecha/models" };

	void prepareFileGUI()
	{
		ZoneScoped;
		scanners.emplace_back( std::make_unique< FileScanner >( path ) );

		//Prepare textures needed.
		folder_texture = Texture::loadFromFile( "./models/folder.png" );
		Sampler sampler { vk::Filter::eLinear,
			              vk::Filter::eLinear,
			              vk::SamplerMipmapMode::eLinear,
			              vk::SamplerAddressMode::eClampToEdge };

		folder_texture->getImageView().getSampler() = std::move( sampler );

		auto cmd_buffer { Device::getInstance().beginSingleTimeCommands() };

		folder_texture->stage( cmd_buffer );

		Device::getInstance().endSingleTimeCommands( cmd_buffer );

		folder_texture->dropStaging();
	}

	void FileBrowser::drawGui( FrameInfo& info )
	{
		ZoneScoped;
		//std::call_once( flag, []() { scanners.emplace_back( std::make_unique< FileScanner >( path ) ); } );
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

		ImGui::Text( "Scanners: %ld", scanners.size() );

		const auto size { ImGui::GetWindowSize() };
		constexpr float desired_size { 128.0f };
		const float extra { std::fmod( size.x, desired_size ) };
		const auto cols { ( size.x - extra ) / desired_size };

		if ( ImGui::BeginTable( "Files", cols ) )
		{
			for ( auto& scanner : scanners )
			{
				//Print out all files found at the inital depth
				for ( const FileInfo& file : *scanner )
				{
					if ( file.depth > 1 ) continue;
					if ( ImGui::TableNextColumn() ) drawFile( file );
				}
			}

			ImGui::EndTable();
		}

		ImGui::Columns( 1 );
	}

	void FileBrowser::drawFile( const FileInfo& data )
	{
		ZoneScoped;
		ImGui::PushID( data.path.c_str() );
		ImGui::Text( data.filename.c_str() );
		ImGui::Text( data.ext.c_str() );

		if ( data.is_folder ) // Folders have no extension
		{
			if ( folder_texture->drawImGuiButton( { 128, 128 } ) )
			{
				std::cout << "Pressed thing" << std::endl;
			}
		}

		ImGui::SameLine();
		ImGui::Text( "%0.1f KB", static_cast< float >( data.size ) / 1024.0f );
		ImGui::NextColumn();
		ImGui::PopID();
	}

} // namespace fgl::engine::filesystem
