//
// Created by kj16609 on 6/3/24.
//

#include "core.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include <imgui_internal.h> // Included for DockBuilder since it's not exposed yet
#pragma GCC diagnostic pop

#include "FileBrowser.hpp"
#include "engine/assets/model/Model.hpp"
#include "engine/debug/DEBUG_NAMES.hpp"
#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/rendering/Renderer.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"
#include "gui_window_names.hpp"
#include "safe_include.hpp"

namespace fgl::engine::gui
{

	inline void prepareDock( ImGuiID& primary_id )
	{
		ImGui::DockBuilderRemoveNode( primary_id );
		ImGui::DockBuilderAddNode( primary_id, ImGuiDockNodeFlags_PassthruCentralNode );

		ImGui::DockBuilderSetNodeSize( primary_id, ImGui::GetMainViewport()->WorkSize );

		constexpr float left_bar_size { 0.2f };
		constexpr float right_bar_size { 0.2f };
		constexpr float bottom_bar_size { 0.4f };

		const ImGuiID lb_node {
			ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Left, left_bar_size, nullptr, &primary_id )
		};

		const ImGuiID rb_node {
			ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Right, right_bar_size, nullptr, &primary_id )
		};

		const ImGuiID bb_node {
			ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Down, bottom_bar_size, nullptr, &primary_id )
		};

		const ImGuiID mv_node { primary_id };
		//ImGuiID mv_node { ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Up, 1.0f - 0.3f, &primary_id, nullptr ) };

		ImGui::DockBuilderDockWindow( OBJECT_TREE_VIEW_NAME, lb_node );

		//ImGui::DockBuilderDockWindow( "Main", mv_node );

		ImGui::DockBuilderDockWindow( CAMERA_EDITOR_NAME, mv_node );

		ImGui::DockBuilderDockWindow( ENTITY_INFO_NAME, rb_node );

		ImGui::DockBuilderDockWindow( FILE_PICKER_NAME, bb_node );

		ImGui::DockBuilderFinish( primary_id );
	}

	// ImGui DockBuilder is still very much not ready for use.
	// But let's try it anyway
	void drawDock()
	{
		// ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

		// Docks seem utterly broken.
		ImGuiID primary_id {
			ImGui::DockSpaceOverViewport( ImGui::GetID( GUI_DOCKSPACE_NAME ), ImGui::GetMainViewport() )
		};
		// +--------------------------------------------------------------------+
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |        |                      mv                          |        |
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |   lb   |                                                  |   rb   |
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |        |--------------------------------------------------|        |
		// |        |                      bb                          |        |
		// |        |                                                  |        |
		// +--------+---------------------------------------------------+-------+

		static std::once_flag flag;
		std::call_once( flag, prepareDock, primary_id );

		ImGui::DockBuilderSetNodePos( primary_id, ImVec2( 0.0f, 0.0f ) );
		ImGui::DockBuilderSetNodeSize( primary_id, ImGui::GetMainViewport()->WorkSize );

		// ImGui::PopStyleVar();
	}

	static GameObject* selected_object { nullptr };

	void itterateGameObjectNode( FrameInfo& info, OctTreeNode& node )
	{
		if ( node.isLeaf() )
		{
			if ( node.itemCount() == 0 ) return;

			auto& objects { node.getLeaf() };
			for ( GameObject& object : objects )
			{
				ImGui::PushID( object.getId() );

				if ( ImGui::TreeNodeEx( object.getName().c_str(), ImGuiTreeNodeFlags_Leaf ) )
				{
					if ( ImGui::IsItemClicked() )
					{
						selected_object = &object;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}
		else if ( node.isBranch() )
		{
			const auto& branches { node.getBranches() };

			for ( std::size_t x = 0; x < 2; ++x )
			{
				for ( std::size_t y = 0; y < 2; ++y )
				{
					for ( std::size_t z = 0; z < 2; ++z )
					{
						const auto item_count { branches[ x ][ y ][ z ]->itemCount() };
						const auto str { std::format( "{}x{}x{}: {}", x, y, z, item_count ) };

						if ( ImGui::TreeNodeEx(
								 str.c_str(),
								 ImGuiTreeNodeFlags_DefaultOpen | ( item_count == 0 ? ImGuiTreeNodeFlags_Leaf : 0 ) ) )
						{
							itterateGameObjectNode( info, *( branches[ x ][ y ][ z ] ) );
							ImGui::TreePop();
						}
					}
				}
			}
		}
	}

	void drawEntityGUI( FrameInfo& info )
	{
		ZoneScoped;
		ImGui::Begin( OBJECT_TREE_VIEW_NAME );

		itterateGameObjectNode( info, info.game_objects );

		/*
		for ( OctTreeNodeLeaf* leaf : info.game_objects.getAllLeafs() )
		{
			for ( GameObject& entity : *leaf )
			{
				ImGui::PushID( entity.getId() );

				if ( ImGui::Selectable( entity.getName().c_str() ) )
				{
					selected_object = &entity;
				}

				ImGui::PopID();
			}
		}
		*/

		ImGui::End();
	}

	void drawEntityInfo( [[maybe_unused]] FrameInfo& info )
	{
		ZoneScoped;
		ImGui::Begin( ENTITY_INFO_NAME );

		if ( !selected_object )
		{
			ImGui::End();
			return;
		}

		drawObject( *selected_object );
		drawComponentsList( *selected_object );

		ImGui::End();
	}

} // namespace fgl::engine::gui
