//
// Created by kj16609 on 6/24/24.
//

#include "preview.hpp"

// clang-format off
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#pragma GCC diagnostic pop
// clang-format on

#include "engine/FrameInfo.hpp"
#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/filesystem/types.hpp"
#include "engine/model/builders/SceneBuilder.hpp"
#include "engine/rendering/SwapChain.hpp"

namespace fgl::engine::gui
{

	void handleDragDrop( FrameInfo& info )
	{
		if ( ImGui::BeginDragDropTarget() )
		{
			if ( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( DRAG_TYPE_FILE_MODEL_INFO ); payload )
			{
				if ( payload->IsDelivery() )
				{
					const filesystem::FileInfo* data { static_cast< filesystem::FileInfo* >( payload->Data ) };

					//Determine what the file is
					switch ( data->engine_type )
					{
						default:
							[[fallthrough]];
						case filesystem::UNKNOWN:
							spdlog::warn( "Unknown filetype dropped into rendering view acceptor" );
							break;
						case filesystem::TEXTURE:
							//Dunno
							break;
						case filesystem::MODEL:
							{
								// Load model and drop it into the game objects
								GameObject obj { GameObject::createGameObject() };

								std::shared_ptr< Model > model {
									Model::createModel( data->path, info.model_vertex_buffer, info.model_index_buffer )
								};

								obj.addFlag( IS_ENTITY | IS_VISIBLE );

								obj.getModel() = std::move( model );

								info.game_objects.addGameObject( std::move( obj ) );

								break;
							}
						case filesystem::SCENE:
							{
								SceneBuilder builder { info.model_vertex_buffer, info.model_index_buffer };

								builder.loadScene( data->path );

								std::vector< GameObject > objs {};
								auto models { builder.getModels() };
								objs.reserve( models.size() );
								for ( auto& model : models )
								{
									GameObject obj { GameObject::createGameObject() };

									obj.getModel() = std::move( model );
									obj.addFlag( IS_ENTITY | IS_VISIBLE );

									objs.emplace_back( std::move( obj ) );
								}

								for ( auto& obj : objs )
								{
									info.game_objects.addGameObject( std::move( obj ) );
								}
							}
					}
				}
				else if ( payload->IsPreview() )
				{
					//TODO: Implement the ability to preview the object in the world before placement
					spdlog::debug( "Payload preview" );
				}
			}

			ImGui::EndDragDropTarget();
		}
	}

	void drawRenderingOutputs( FrameInfo& info )
	{
		ZoneScoped;
		ImGui::Begin( "RenderOutputs" );

		enum RenderingOutputSelection : std::uint_fast8_t
		{
			Composite = 0,
			Albedo = 1,
			Normal = 2,
			Position = 3
		};

		static const char* const options[] = { "Composite", "Albedo", "Normal", "Position" };
		static std::uint_fast8_t current { Composite };

		if ( ImGui::BeginCombo( "Rendering Output", options[ current ] ) )
		{
			constexpr float desired_size { 64 };
			//Calculate size
			const float ratio { info.swap_chain.extentAspectRatio() };

			// h = w/h

			float fh_size { desired_size };
			float fv_size { desired_size * ratio };

			// If height is larger then the size then we need to compute the width from the height max
			if ( fv_size > desired_size )
			{
				fv_size = desired_size;
				fh_size = fv_size / ratio;
			}

			std::uint32_t h_size { static_cast< std::uint32_t >( fh_size ) };
			std::uint32_t v_size { static_cast< std::uint32_t >( fv_size ) };

			//Composite
			if ( ImGui::Selectable( options[ Composite ], current == Composite ) )
			{
				log::debug( "Changing output to Compositite" );
				current = Composite;
			}

			info.swap_chain.g_buffer_albedo_img->drawImGui( { v_size, h_size } );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Albedo ], current == Albedo ) )
			{
				log::debug( "Changing output to Albedo" );
				current = Albedo;
			}

			info.swap_chain.g_buffer_normal_img->drawImGui( { v_size, h_size } );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Normal ], current == Normal ) )
			{
				log::debug( "Changing output to Normal" );
				current = Normal;
			}

			info.swap_chain.g_buffer_position_img->drawImGui( { v_size, h_size } );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Position ], current == Position ) )
			{
				log::debug( "Changing output to Position" );
				current = Position;
			}

			ImGui::EndCombo();
		}

		switch ( current )
		{
			default:
				[[fallthrough]];
			case Composite:
				info.swap_chain.g_buffer_composite_img->drawImGui();
				break;
			case Albedo:
				info.swap_chain.g_buffer_albedo_img->drawImGui();
				break;
			case Normal:
				info.swap_chain.g_buffer_normal_img->drawImGui();
				break;
			case Position:
				info.swap_chain.g_buffer_position_img->drawImGui();
				break;
		}

		handleDragDrop( info );

		ImGui::End();
	}

} // namespace fgl::engine::gui
