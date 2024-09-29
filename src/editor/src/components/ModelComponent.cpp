//
// Created by kj16609 on 8/2/24.
//

#include "engine/gameobjects/components/ModelComponent.hpp"

#include "engine/assets/model/Model.hpp"
#include "engine/gameobjects/components/drawers.hpp"
#include "gui/safe_include.hpp"

namespace fgl::engine
{

#ifdef TITOR_EDITOR
	void ModelComponent::drawImGui()
	{
		drawComponentTransform( m_transform );

		//ImGui::Text( "MODEL COMPONENT WOOOOOO" );

		// TODO: If the model is not set then we should be able to set it to one from the file selection
		if ( this->m_model == nullptr )
		{
			ImGui::Text( "Undefined model" );
			return;
		}

		const auto& model { *this->m_model };

		ImGui::Text( "%i primitives", model.m_primitives.size() );
	}

	std::string_view ModelComponent::name() const
	{
		if ( !m_model ) return "Empty";

		return m_model->getName();
	}
#endif

	Model* ModelComponent::operator->()
	{
		return m_model.get();
	}

	const Model* ModelComponent::operator->() const
	{
		return m_model.get();
	}
} // namespace fgl::engine
