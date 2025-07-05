//
// Created by kj16609 on 4/5/25.
//
#include "engine/gameobjects/components/TransformComponent.hpp"

#include "engine/assets/model/Model.hpp"
#include "gui/safe_include.hpp"

namespace fgl::engine::components
{
	void TransformComponent::triggerUpdate()
	{
		auto visitorFunc = [ & ]< typename T0 >( T0& to_update )
		{
			if constexpr ( std::same_as< T0, std::weak_ptr< ModelInstance > > )
			{
				if ( const std::shared_ptr< ModelInstance > model_instance = to_update.lock() )
				{
					model_instance->setTransform( m_transform );
					model_instance->flagUpdate();
				}
			}
		};

		for ( auto& to_update : m_updatables )
		{
			std::visit( visitorFunc, to_update );
		}
	}

	TransformComponent::TransformComponent()
	{}

	TransformComponent::TransformComponent( const WorldTransform& transform ) : m_transform( transform )
	{}

	void TransformComponent::drawImGui()
	{
		glm::vec3& position = m_transform.translation.vec();

		if ( ImGui::DragFloat3( "Position", &position.x, 0.1f ) )
		{
			triggerUpdate();
		}

		if ( m_transform.rotation.isEuler() )
		{
			glm::vec3& rotation = m_transform.rotation.euler().vec();
			if ( ImGui::DragFloat3( "Rotation", &rotation.x, 0.1f ) )
			{
				// Clamp high
				while ( rotation.x > 180.0f ) rotation.x -= 360.0f;
				while ( rotation.y > 180.0f ) rotation.y -= 360.0f;
				while ( rotation.z > 180.0f ) rotation.z -= 360.0f;
				// Clamp low
				while ( rotation.x < -180.0f ) rotation.x += 360.0f;
				while ( rotation.y < -180.0f ) rotation.y += 360.0f;
				while ( rotation.z < -180.0f ) rotation.z += 360.0f;

				triggerUpdate();
			}
		}

		glm::vec3& scale = m_transform.scale;
		if ( ImGui::DragFloat3( "Scale", &scale.x, 0.1f ) )
		{
			triggerUpdate();
		}
	}

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

	void TransformComponent::setTransform( const WorldTransform& transform )
	{
		m_transform = transform;
	}

	TransformComponent::~TransformComponent()
	{}
} // namespace fgl::engine::components