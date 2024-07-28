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
#include "engine/camera/Camera.hpp"
#include "engine/camera/CameraSwapchain.hpp"
#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/filesystem/types.hpp"
#include "engine/gameobjects/components/ModelComponent.hpp"
#include "engine/model/Model.hpp"
#include "engine/model/builders/SceneBuilder.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

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
						case filesystem::BINARY:
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

								std::unique_ptr< ModelComponent > component {
									std::make_unique< ModelComponent >( std::move( model ) )
								};

								obj.addComponent( std::move( component ) );

								info.game_objects.addGameObject( std::move( obj ) );

								break;
							}
						case filesystem::SCENE:
							{
								SceneBuilder builder { info.model_vertex_buffer, info.model_index_buffer };

								builder.loadScene( data->path );

								std::vector< GameObject > objs { builder.getGameObjects() };

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

	vk::Extent2D calculateTargetSize( const float ratio, const vk::Extent2D max_extent )
	{
		// r = w/h
		// w = r*h
		// h = w/r
		float height_size { static_cast< float >( max_extent.height ) };
		float width_size { ratio * static_cast< float >( max_extent.height ) };

		// If height is larger then the size then we need to compute the width from the height max
		if ( width_size > max_extent.width )
		{
			width_size = static_cast< float >( max_extent.width );
			height_size = static_cast< float >( max_extent.width ) / ratio;
		}

		return { static_cast< std::uint32_t >( width_size ), static_cast< std::uint32_t >( height_size ) };
	}

	FGL_FORCE_INLINE_FLATTEN vk::Extent2D calculateTargetSize( const float ratio, const ImVec2 max_extent )
	{
		return calculateTargetSize( ratio, vk::Extent2D( max_extent.x, max_extent.y ) );
	}

	void drawRenderingOutputs( FrameInfo& info, const Camera& camera )
	{
		ZoneScoped;
		const auto frame_index { info.frame_idx };

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
			constexpr vk::Extent2D desired_size { 64, 64 };
			//Calculate size
			const float ratio { info.swap_chain.extentAspectRatio() };
			const auto size { calculateTargetSize( ratio, desired_size ) };

			//Composite
			if ( ImGui::Selectable( options[ Composite ], current == Composite ) )
			{
				log::debug( "Changing output to Compositite" );
				current = Composite;
			}

			camera.getSwapchain().g_buffer_albedo_img[ frame_index ]->drawImGui( size );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Albedo ], current == Albedo ) )
			{
				log::debug( "Changing output to Albedo" );
				current = Albedo;
			}

			camera.getSwapchain().g_buffer_normal_img[ frame_index ]->drawImGui( size );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Normal ], current == Normal ) )
			{
				log::debug( "Changing output to Normal" );
				current = Normal;
			}

			camera.getSwapchain().g_buffer_position_img[ frame_index ]->drawImGui( size );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Position ], current == Position ) )
			{
				log::debug( "Changing output to Position" );
				current = Position;
			}

			ImGui::EndCombo();
		}

		const float ratio { info.swap_chain.extentAspectRatio() };
		const auto imgui_size { ImGui::GetWindowSize() };
		const auto target_size { calculateTargetSize( ratio, imgui_size ) };

		//Compute optimal size using aspect ratio

		switch ( current )
		{
			default:
				[[fallthrough]];
			case Composite:
				camera.getSwapchain().g_buffer_composite_img[ frame_index ]->drawImGui( target_size );
				break;
			case Albedo:
				camera.getSwapchain().g_buffer_albedo_img[ frame_index ]->drawImGui( target_size );
				break;
			case Normal:
				camera.getSwapchain().g_buffer_normal_img[ frame_index ]->drawImGui( target_size );
				break;
			case Position:
				camera.getSwapchain().g_buffer_position_img[ frame_index ]->drawImGui( target_size );
				break;
		}

		handleDragDrop( info );
	}

} // namespace fgl::engine::gui
