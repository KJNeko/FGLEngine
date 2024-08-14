//
// Created by kj16609 on 8/13/24.
//

#include "GameObjectComponent.hpp"

#include <imgui.h>

namespace fgl::engine
{
	constexpr auto indent_amount { 1.0f };

	void GameObjectComponentBase::drawNode( GameObjectComponentPtr& selected_out )
	{
		ImGui::Indent( indent_amount );
		if ( ImGui::Selectable( this->name().data() ) )
		{
			selected_out = this;
		}

		drawChildren( selected_out );
		ImGui::Unindent( indent_amount );
	}

	void GameObjectComponentBase::drawChildren( GameObjectComponentPtr& selected_out )
	{
		for ( auto* child : m_children )
		{
			child->drawNode( selected_out );
		}
	}

} // namespace fgl::engine