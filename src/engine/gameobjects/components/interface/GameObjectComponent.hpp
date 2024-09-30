//
// Created by kj16609 on 7/7/24.
//

#pragma once

#include <vector>

#include "ComponentEngineInterface.hpp"
#include "engine/primitives/TransformComponent.hpp"

#define COMPONENT_CLASS( class_name, id_name ) class class_name final : public GameObjectComponent< id_name >

namespace fgl::engine
{

	struct ComponentTransform final : public TransformComponent< CoordinateSpace::World >
	{
		enum Mode
		{
			//! Object is non moving, When this is set only the transform on the component should be used.
			Static,
			//! Object moves in relation to it's parent
			Local,
			//! Object moves in relation to the world
			Global
		} m_mode { Global };

		ComponentTransform() = default;

		ComponentTransform( const WorldCoordinate position_i, const Scale scale_i, const Rotation rotation_i ) :
		  TransformComponent( position_i, scale_i, rotation_i )
		{}
	};

	struct GameObjectComponentBase : public ComponentEditorInterface, public ComponentEngineInterface
	{
		std::vector< GameObjectComponentBase* > m_children {};

		void drawNode( GameObjectComponentBase*& selected_out );

		void drawChildren( GameObjectComponentBase*& selected_out );
	};

	using GameObjectComponentPtr = GameObjectComponentBase*;

	//TODO: Add way to add components to an object in the editor.

	template < ComponentEngineInterface::ComponentID T_ID >
	struct GameObjectComponent : public GameObjectComponentBase
	{
		constexpr static ComponentID ID { T_ID };
		ComponentTransform m_transform { WorldCoordinate( 0.0f ), Scale( 1.0 ), Rotation() };

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