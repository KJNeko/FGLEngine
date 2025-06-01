//
// Created by kj16609 on 4/5/25.
//
#include "engine/gameobjects/components/TransformComponent.hpp"

#include "engine/assets/model/Model.hpp"
#include "gui/safe_include.hpp"

namespace fgl::engine::components
{
	TransformComponent::TransformComponent() : m_transform(), m_model_instance_info_index()
	{}

	TransformComponent::TransformComponent( const WorldTransform& transform ) :
	  m_transform( transform ),
	  m_model_instance_info_index()
	{}

	void TransformComponent::drawImGui()
	{}

	std::string_view TransformComponent::humanName() const
	{
		return "Transform";
	}

	std::string_view TransformComponent::className() const
	{
		return "TransformComponent";
	}

	WorldTransform& TransformComponent::operator*()
	{
		return m_transform;
	}
} // namespace fgl::engine::components