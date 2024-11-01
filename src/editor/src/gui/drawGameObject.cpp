//
// Created by kj16609 on 8/13/24.
//

#include "engine/gameobjects/GameObject.hpp"
#include "gui/helpers.hpp"

namespace fgl::engine::gui
{
	void drawObject( GameObject& game_object )
	{
		static std::string name_input_temp {};
		name_input_temp = game_object.getName();
		ImGui::InputText( "Name", &name_input_temp );
		if ( game_object.getName() != name_input_temp ) game_object.setName( name_input_temp );

		// Transform - Position
		dragFloat3( "Position", game_object.getTransform().translation.vec() );

		dragFloat3Rot( "Rotation", game_object.getRotation() );

		dragFloat3( "Scale", game_object.getScale() );
	}

	static GameObjectComponentPtr SELECTED_COMPONENT { nullptr };

	void drawComponentsList( GameObject& game_object )
	{
		ImGui::SeparatorText( "Components" );

		for ( const GameObjectComponentPtr component : game_object.getComponents() )
		{
			component->drawNode( SELECTED_COMPONENT );
		}

		if ( SELECTED_COMPONENT != nullptr )
		{
			ImGui::SeparatorText( "Selected Component" );

			ImGui::PushID( "ComponentEditor" );
			SELECTED_COMPONENT->drawImGui();
			ImGui::PopID();
		}
	}

} // namespace fgl::engine::gui