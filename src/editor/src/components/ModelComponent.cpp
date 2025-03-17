//
// Created by kj16609 on 8/2/24.
//

#include "engine/gameobjects/components/ModelComponent.hpp"

#include "engine/assets/model/Model.hpp"
#include "gui/safe_include.hpp"

namespace fgl::engine::components
{

	void ModelComponent::drawImGui()
	{
		// drawComponentTransform( m_transform );

		// TODO: If the model is not set then we should be able to set it to one from the file selection
		if ( this->m_model == nullptr )
		{
			ImGui::Text( "Undefined model" );
			return;
		}

		const auto& model { *this->m_model };

		ImGui::Text( "%li primitives", model.m_primitives.size() );
	}

	std::string_view ModelComponent::humanName() const
	{
		if ( !m_model ) return "Empty";

		return m_model->getName();
	}

	std::string_view ModelComponent::className() const
	{
		return "ModelComponent";
	}

	Model* ModelComponent::operator->()
	{
		return m_model.get();
	}

	const Model* ModelComponent::operator->() const
	{
		return m_model.get();
	}
} // namespace fgl::engine
