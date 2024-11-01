//
// Created by kj16609 on 8/13/24.
//

#include "GameObjectComponent.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <imgui.h>
#pragma GCC diagnostic pop

namespace fgl::engine
{
	constexpr auto INDENT_AMOUNT { 1.0f };

	void GameObjectComponentBase::drawNode( GameObjectComponentPtr& selected_out )
	{
		ImGui::Indent( INDENT_AMOUNT );
		if ( ImGui::Selectable( this->name().data() ) )
		{
			selected_out = this;
		}

		drawChildren( selected_out );
		ImGui::Unindent( INDENT_AMOUNT );
	}

	void GameObjectComponentBase::drawChildren( GameObjectComponentPtr& selected_out )
	{
		for ( auto* child : m_children )
		{
			child->drawNode( selected_out );
		}
	}

} // namespace fgl::engine