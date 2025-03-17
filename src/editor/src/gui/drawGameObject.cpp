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

		/*
		auto& transform { game_object.getTransform() };

		// Transform - Position
		dragFloat3( "Position", transform.translation.vec() );

		if ( transform.rotation.isEuler() )
		{
			dragFloat3Rot( "Rotation", transform.rotation.euler() );
		}
		else
		{
			ImGui::Text( "Rotation was in quaternion" );
		}

		dragFloat3( "Scale", transform.scale );
		*/
	}

	static GameObjectComponentPtr SELECTED_COMPONENT { nullptr };

	void drawComponentsList( GameObject& game_object )
	{
		ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 5.0f );
		ImGui::BeginChild(
			"ComponentsList",
			ImVec2( 0, 0 ),
			ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY );

		ImGui::SeparatorText( "Components" );

		const auto& components { game_object.getComponents() };

		for ( const GameObjectComponentPtr component : components )
		{
			component->drawNode( SELECTED_COMPONENT );
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::BeginChild( "Selected", ImVec2( 0, 0 ), ImGuiChildFlags_Border );

		if ( SELECTED_COMPONENT != nullptr && std::ranges::find( components, SELECTED_COMPONENT ) != components.end() )
		{
			ImGui::SeparatorText( "Selected Component" );

			ImGui::PushID( "ComponentEditor" );
			SELECTED_COMPONENT->drawImGui();
			ImGui::PopID();
		}
		else
		{
			ImGui::SeparatorText( "No component selected" );
		}

		ImGui::EndChild();
	}

} // namespace fgl::engine::gui