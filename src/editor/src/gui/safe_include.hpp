//
// Created by kj16609 on 6/5/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
// clang-format off
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
// clang-format on
#pragma GCC diagnostic pop

#include <string_view>

#include "engine/FGL_DEFINES.hpp"

namespace ImGui
{

	//void ImGui::DockBuilderDockWindow(const char* window_name, ImGuiID node_id)
	FGL_FORCE_INLINE inline void DockBuilderDockWindow( const std::string_view str, ImGuiID node_id )
	{
		::ImGui::DockBuilderDockWindow( str.data(), node_id );
	}

	// IMGUI_API ImGuiID       GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into ImGuiStorage yourself
	FGL_FORCE_INLINE inline ImGuiID GetID( const std::string_view str )
	{
		return ::ImGui::GetID( str.data() );
	}

	// IMGUI_API bool          Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
	FGL_FORCE_INLINE inline bool
		Begin( const std::string_view name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0 )
	{
		return ::ImGui::Begin( name.data(), p_open, flags );
	}

	// IMGUI_API bool          MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
	FGL_FORCE_INLINE inline bool MenuItem(
		const std::string_view label, const char* shortcut = NULL, bool selected = false, bool enabled = true )
	{
		return ::ImGui::MenuItem( label.data(), shortcut, selected, enabled );
	}

	// IMGUI_API bool          MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL
	FGL_FORCE_INLINE inline bool
		MenuItem( const std::string_view label, const char* shortcut, bool* selected, bool enabled = true )
	{
		return ::ImGui::MenuItem( label.data(), shortcut, selected, enabled );
	}

} // namespace ImGui
