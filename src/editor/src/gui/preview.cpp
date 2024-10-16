//
// Created by kj16609 on 6/24/24.
//

#include "preview.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/assets/model/Model.hpp"
#include "engine/assets/model/builders/SceneBuilder.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/camera/CameraSwapchain.hpp"
#include "engine/filesystem/scanner/FileScanner.hpp"
#include "engine/filesystem/types.hpp"
#include "engine/gameobjects/components/ModelComponent.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"
#include "safe_include.hpp"

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

		// If the width is higher then the max extent, Then we wanna use the width instead
		if ( width_size > max_extent.width )
		{
			width_size = static_cast< float >( max_extent.width );
			height_size = static_cast< float >( max_extent.width ) / ratio;
		}

		assert( width_size <= max_extent.width );
		assert( height_size <= max_extent.height );

		return { static_cast< std::uint32_t >( width_size ), static_cast< std::uint32_t >( height_size ) };
	}

	FGL_FORCE_INLINE_FLATTEN inline vk::Extent2D calculateTargetSize( const float ratio, const ImVec2 max_extent )
	{
		return calculateTargetSize( ratio, vk::Extent2D( max_extent.x, max_extent.y ) );
	}

	enum RenderingOutputSelection : std::uint_fast8_t
	{
		Composite = 0,
		Albedo = 1,
		Normal = 2,
		Position = 3
	};

	inline static bool hotkeys_enabled { true };

	inline void drawConfigBar(
		[[maybe_unused]] const FrameInfo& info,
		[[maybe_unused]] const Camera& camera,
		[[maybe_unused]] const FrameIndex frame_index,
		std::uint_fast8_t& current )
	{
		static constexpr std::string_view options[] = { "Composite", "Albedo", "Normal", "Position" };
		if ( ImGui::BeginMenuBar() )
		{
			if ( ImGui::BeginMenu( "Output" ) )
			{
				if ( ImGui::MenuItem( options[ Composite ], "Alt+1", current == Composite, true ) )
				{
					current = Composite;
				}

				if ( ImGui::MenuItem( options[ Albedo ], "Alt+2", current == Albedo, true ) )
				{
					current = Albedo;
				}

				if ( ImGui::MenuItem( options[ Normal ], "Alt+3", current == Normal, true ) )
				{
					current = Normal;
				}

				if ( ImGui::MenuItem( options[ Position ], "Alt+4", current == Position, true ) )
				{
					current = Position;
				}

				if ( ImGui::MenuItem( "Enable Hotkeys" ), nullptr, hotkeys_enabled )
				{
					hotkeys_enabled = !hotkeys_enabled;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void drawRenderingOutputs( FrameInfo& info, const Camera& camera )
	{
		ZoneScoped;
		const auto frame_index { info.frame_idx };

		static std::uint_fast8_t current { Composite };

		drawConfigBar( info, camera, frame_index, current );

		const float ratio { camera.aspectRatio() };
		auto imgui_size { ImGui::GetWindowSize() };
		imgui_size.x -= 8;

		const auto target_size { calculateTargetSize( ratio, imgui_size ) };

		if ( hotkeys_enabled && ImGui::IsWindowFocused() && ImGui::IsKeyDown( ImGuiKey_LeftAlt ) )
		{
			if ( ImGui::IsKeyPressed( ImGuiKey_1 ) )
			{
				current = Composite;
			}
			else if ( ImGui::IsKeyPressed( ImGuiKey_2 ) )
			{
				current = Albedo;
			}
			else if ( ImGui::IsKeyPressed( ImGuiKey_3 ) )
			{
				current = Normal;
			}
			else if ( ImGui::IsKeyPressed( ImGuiKey_4 ) )
			{
				current = Position;
			}
		}

		switch ( current )
		{
			default:
				[[fallthrough]];
			case Composite:
				camera.getSwapchain().g_buffer_composite_img[ frame_index ]->drawImGui( target_size );
				break;
			case Albedo:
				camera.getSwapchain().g_buffer_color_img[ frame_index ]->drawImGui( target_size );
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
