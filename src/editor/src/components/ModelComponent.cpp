//
// Created by kj16609 on 8/2/24.
//

#include "engine/gameobjects/components/ModelComponent.hpp"

#include "engine/gameobjects/components/drawers.hpp"
#include "engine/model/Model.hpp"
#include "gui/safe_include.hpp"

namespace fgl::engine
{

#ifdef TITOR_EDITOR
	void ModelComponent::drawImGui()
	{
		drawComponentTransform( m_transform );

		ImGui::Text( "MODEL COMPONENT WOOOOOO" );
	}

	std::string_view ModelComponent::name() const
	{
		if ( m_model )
			return m_model->getName();
		else
			return "Empty Model";
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
