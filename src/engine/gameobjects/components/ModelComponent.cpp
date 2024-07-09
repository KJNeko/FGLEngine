//
// Created by kj16609 on 7/8/24.
//

#include "ModelComponent.hpp"

#include <imgui.h>

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
