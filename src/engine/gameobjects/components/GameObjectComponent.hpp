//
// Created by kj16609 on 7/7/24.
//

#pragma once

#include "ComponentEngineInterface.hpp"
#include "engine/primitives/TransformComponent.hpp"

namespace fgl::engine
{
	using ComponentTransform = TransformComponent;

	struct GameObjectComponentBase : public ComponentEditorInterface, public ComponentEngineInterface
	{};

	using GameObjectComponentPtr = GameObjectComponentBase*;

	template < ComponentEngineInterface::ComponentID T_ID >
	struct GameObjectComponent : public GameObjectComponentBase
	{
		constexpr static ComponentID ID { T_ID };
		ComponentTransform m_transform;

		virtual ComponentID id() const override final { return ID; }
	};

	template < typename T >
	concept is_component = requires( T t ) {
		std::is_base_of_v< T, ComponentEngineInterface >;
		{
			t.ID
		} -> std::same_as< const ComponentEngineInterface::ComponentID& >;
	};

} // namespace fgl::engine