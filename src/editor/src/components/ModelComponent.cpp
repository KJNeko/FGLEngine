//
// Created by kj16609 on 8/2/24.
//

#include "engine/gameobjects/components/ModelComponent.hpp"

#include "engine/assets/model/Model.hpp"
#include "gui/safe_include.hpp"

namespace fgl::engine::components
{

	ModelComponent::ModelComponent( const std::shared_ptr< Model >& model ) :
	  m_model_instance( model->createInstance() ),
	  m_transform()
	{}

	void ModelComponent::drawImGui()
	{
		ImGui::Text( "Primitives: " );
		ImGui::SameLine();
		ImGui::Text( "%li", m_model_instance->m_model->m_primitives.size() );

		// list each primitive and list the buffer that it is stored in
		for ( const auto& primitive : m_model_instance->m_model->m_primitives )
		{
			ImGui::Text( "Primitive: %s", primitive.m_name.c_str() );
			ImGui::SameLine();
			ImGui::Text( "Buffer: %s", primitive.m_vertex_buffer.getBuffer()->sizeName().c_str() );
		}

		// drawComponentTransform( m_transform );

		// TODO: If the model is not set then we should be able to set it to one from the file selection
		/*
		if ( this->m_model == nullptr )
		{
			ImGui::Text( "Undefined model" );
			return;
		}

		const auto& model { *this->m_model };

		ImGui::Text( "%li primitives", model.m_primitives.size() );
		*/
	}

	std::string_view ModelComponent::humanName() const
	{
		if ( !m_model_instance ) return "Empty";
		return "TODO";
		// return m_model_instance->getModel()->getName();
	}

	std::string_view ModelComponent::className() const
	{
		return "ModelComponent";
	}

	/*
	Model* ModelComponent::operator->()
	{
		return m_model.get();
	}

	const Model* ModelComponent::operator->() const
	{
		return m_model.get();
	}
	*/
} // namespace fgl::engine::components
