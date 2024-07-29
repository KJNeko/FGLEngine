//
// Created by kj16609 on 7/28/24.
//

#pragma once


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include <imgui.h>
#pragma GCC diagnostic pop

namespace ImGui
{
	ImGuiID DockSpaceOverViewportEx(const ImGuiViewport* viewport, ImGuiDockNodeFlags dockspace_flags, const ImGuiWindowClass* window_class, auto DockBuilder)
	{
		if (viewport == NULL)
			viewport = GetMainViewport();

		SetNextWindowPos(viewport->WorkPos);
		SetNextWindowSize(viewport->WorkSize);
		SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags host_window_flags = 0;
		host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
		host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			host_window_flags |= ImGuiWindowFlags_NoBackground;

		char label[32];
		ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);

		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		Begin(label, NULL, host_window_flags);
		PopStyleVar(3);

		ImGuiID dockspace_id = GetID("DockSpace");

		/// vvvv ---- call DockBuilder callback if no docking exists --- vvvv
		if(DockBuilderGetNode(dockspace_id) == nullptr)
		{
			DockBuilder(dockspace_id);
		}
		/// ^^^^ ---------------------------------------------------- ^^^^

		DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, window_class);
		End();

		return dockspace_id;
	}
}
