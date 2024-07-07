//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "components/GameObjectComponent.hpp"
#include "engine/primitives/TransformComponent.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct OrientedBoundingBox;

	class Model;

	using GameObjectFlagType = std::uint16_t;

	enum GameObjectFlagMask : GameObjectFlagType
	{
		NONE_FLAG = 0,
		IS_STATIC = 1 << 0, //! Object can not move
		IS_VISIBLE = 1 << 1, //! Object is visible
		IS_TERRAIN = 1 << 2, //! Object is a part of the terrain
		HAS_MODEL = 1 << 3,
		IS_ENTITY = 1 << 4,
		MASK_DEFAULT = IS_VISIBLE,
	};

	enum GameObjectFilterOptions
	{
		NONE = 0,
		//! Only find objects with no texture
		TEXTURELESS = 1 << 0,
	};

	class GameObject
	{
	  public:

		using GameObjectID = unsigned int;
		using Map = std::unordered_map< GameObjectID, GameObject >;

		static constexpr GameObjectID INVALID_ID { std::numeric_limits< GameObjectID >::max() };

	  private:

		GameObjectID m_id { INVALID_ID };
		GameObjectFlagType object_flags { GameObjectFlagMask::MASK_DEFAULT };
		TransformComponent m_transform {};

		std::vector< GameObjectComponentBase* > components {};

		std::shared_ptr< Model > m_model { nullptr };
		std::string name {};

		explicit GameObject( GameObjectID obj_id ) : m_id( obj_id ) {}

		FGL_DELETE_DEFAULT_CTOR( GameObject );
		FGL_DELETE_COPY( GameObject );

	  public:

		GameObject& operator=( GameObject&& other ) = default;
		GameObject( GameObject&& other ) = default;

		template < typename T >
			requires is_component< T >
		bool hasComponent()
		{
			for ( const GameObjectComponentBase* comp : components )
			{
				if ( comp->id() == T::ID ) return true;
			}

			return false;
		}

		//Flags
		GameObjectFlagType flags() const { return object_flags; }

		void addFlag( GameObjectFlagType flag ) { object_flags |= flag; }

		void removeFlag( GameObjectFlagType flag ) { object_flags &= ( ~flag ); }

		//Model
		bool hasModel() const { return m_model != nullptr; }

		const std::shared_ptr< Model >& getModel() const { return m_model; }

		std::shared_ptr< Model >& getModel() { return m_model; }

		//Transform
		TransformComponent& getTransform() { return m_transform; }

		const TransformComponent& getTransform() const { return m_transform; }

		const WorldCoordinate& getPosition() const { return m_transform.translation; }

		const Rotation& getRotation() const { return m_transform.rotation; }

		//Bounding Box
		OrientedBoundingBox< CoordinateSpace::World > getBoundingBox() const;

		//Misc
		static GameObject createGameObject();

		GameObjectID getId() const { return m_id; }

		//! Returns the name of the game object. If no name is set then the name of the model is used.
		std::string& getName() { return name; }

		void drawImGui();
	};

	//	static_assert( offsetof( GameObject, hot_limter ) < 64, "Hot limit reached" );

} // namespace fgl::engine
