//
// Created by kj16609 on 8/2/24.
//

#include "engine/gameobjects/components/ModelComponent.hpp"

#include "gui/safe_include.hpp"

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
