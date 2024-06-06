//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>
#include <unordered_map>

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

	struct ComponentImGuiInterface
	{
		virtual void drawImGui() = 0;
		virtual ~ComponentImGuiInterface() = default;
	};

	struct GameObjectComponentBase
	{
		using ComponentID = std::uint8_t;
		virtual ComponentID id() const = 0;
		virtual std::string_view name() const = 0;

		virtual ~GameObjectComponentBase() = default;
	};

	template < GameObjectComponentBase::ComponentID T_ID >
	struct GameObjectComponent : ComponentImGuiInterface, GameObjectComponentBase
	{
		constexpr static ComponentID ID { T_ID };

		virtual ComponentID id() const override final { return ID; }
	};

	template < typename T >
	concept is_component = requires( T t ) {
		std::is_base_of_v< T, GameObjectComponentBase >;
		{
			t.ID
		} -> std::same_as< GameObjectComponentBase::ComponentID >;
	};

	class ModelComponent final : public GameObjectComponent< 1 >
	{
		std::shared_ptr<Model> m_model;

	  public:

		void drawImGui() override;
		std::string_view name() const override;
		~ModelComponent() override;
	};

	class GameObject
	{
	  public:

		using GameObjectID = unsigned int;
		using Map = std::unordered_map< GameObjectID, GameObject >;

		static constexpr GameObjectID INVALID_ID { std::numeric_limits< GameObjectID >::max() };

		GameObjectID m_id { INVALID_ID };
		GameObjectFlagType object_flags { GameObjectFlagMask::MASK_DEFAULT };
		TransformComponent m_transform {};

		std::vector< GameObjectComponentBase* > components {};

		std::shared_ptr< Model > m_model { nullptr };
		std::string name { "Unnamed Game Object" };

	  private:

		GameObject( GameObjectID obj_id ) : m_id( obj_id ) {}

		GameObject() = delete;

	  public:

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

		GameObject( const GameObject& other ) = delete;
		GameObject& operator=( const GameObject& other ) = delete;

		GameObject( GameObject&& other ) = default;
		GameObject& operator=( GameObject&& ) = default;

		inline const WorldCoordinate& getPosition() const { return m_transform.translation; }

		inline const Rotation& getRotation() const { return m_transform.rotation; }

		OrientedBoundingBox< CoordinateSpace::World > getBoundingBox() const;

		static GameObject createGameObject();

		inline GameObjectID getId() const { return m_id; }

		void drawImGui();
	};

	//	static_assert( offsetof( GameObject, hot_limter ) < 64, "Hot limit reached" );

} // namespace fgl::engine
