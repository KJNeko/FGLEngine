//
// Created by kj16609 on 8/13/24.
//

#include "drawers.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <imgui.h>

#include "editor/src/gui/helpers.hpp"
#pragma GCC diagnostic pop

namespace fgl::engine
{

	void drawComponentTransform( ComponentTransform& transform )
	{
		if ( ImGui::CollapsingHeader( "Transform" ) )
		{
			constexpr float speed { 1.0f };

			ImGui::DragFloat3( "Position", &transform.translation.x, speed );

			gui::dragFloat3Rot( "Rotation", transform.rotation );

			ImGui::DragFloat3( "Scale", &transform.scale.x, speed );
		}
	}

} // namespace fgl::engine
