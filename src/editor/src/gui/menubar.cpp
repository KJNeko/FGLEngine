//
// Created by kj16609 on 1/29/25.
//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include <imgui.h>
#pragma GCC diagnostic pop

#include "FrameInfo.hpp"

namespace fgl::engine::gui
{

	void drawMenubar( [[maybe_unused]] FrameInfo& info )
	{
		ImGui::BeginMainMenuBar();

		if ( ImGui::BeginMenu( "File" ) )
		{
			if ( ImGui::MenuItem( "Save..." ) )
			{}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

} // namespace fgl::engine::gui
