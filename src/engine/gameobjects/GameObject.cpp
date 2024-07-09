//
// Created by kj16609 on 11/30/23.
//

#include "GameObject.hpp"

#include "engine/gui/helpers.hpp"
#include "engine/gui/safe_include.hpp"
#include "engine/model/Model.hpp"

namespace fgl::engine
{

	GameObject GameObject::createGameObject()
	{
		static GameObjectID current_id { 0 };
		return GameObject( current_id++ );
	}

	void GameObject::drawImGui()
	{
		ImGui::InputText( "Name", &( this->getName() ) );

		// Transform - Position
		WorldCoordinate& translation { this->m_transform.translation };
		gui::dragFloat3( "Position", translation.vec() );

		Rotation& rotation { this->m_transform.rotation };
		gui::dragFloat3Rot( "Rotation", rotation );

		auto& scale { this->m_transform.scale };
		gui::dragFloat3( "Scale", scale );

		for ( ComponentImGuiInterface* component : components )
		{
			ImGui::Separator();
			component->drawImGui();
		}
	}

} // namespace fgl::engine