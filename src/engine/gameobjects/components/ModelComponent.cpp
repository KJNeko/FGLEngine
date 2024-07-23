//
// Created by kj16609 on 7/8/24.
//

#include "ModelComponent.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include <imgui.h>
#pragma GCC diagnostic pop

namespace fgl::engine
{

	void ModelComponent::drawImGui()
	{
		if ( ImGui::CollapsingHeader( "Model Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			ImGui::Text( "MODEL COMPONENT WOOOOOO" );
		}
	}

} // namespace fgl::engine
