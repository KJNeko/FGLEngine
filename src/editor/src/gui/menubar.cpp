//
// Created by kj16609 on 1/29/25.
//

#include <imgui.h>

#include "FrameInfo.hpp"

namespace fgl::engine::gui
{

	void drawMenubar( FrameInfo& info )
	{
		ImGui::BeginMainMenuBar();

		if ( ImGui::BeginMenu( "File" ) )
		{
			if ( ImGui::MenuItem( "Save..." ) )
			{



			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

} // namespace fgl::engine::gui
