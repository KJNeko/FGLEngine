//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <tracy/Tracy.hpp>

#include <unordered_map>
#include <vector>

#include "components/ModelComponent.hpp"
#include "engine/gameobjects/components/interface/GameObjectComponent.hpp"
#include "engine/primitives/TransformComponent.hpp"

namespace fgl::engine
{
	struct Scale;
	template < CoordinateSpace CType >
	class OrientedBoundingBox;

	class Model;

	using GameObjectFlagType = std::uint16_t;

	enum GameObjectFlagMask : GameObjectFlagType
	{
		NoneFlag = 0,
		IsStatic = 1 << 0, //! Object can not move
		IsVisible = 1 << 1, //! Only return visible objects
		IsEntity = 1 << 2,
		MaskDefault = IsVisible,
	};

	class GameObject
	{
	  public:

		using GameObjectID = unsigned int;
		using Map = std::unordered_map< GameObjectID, GameObject >;

		static constexpr GameObjectID INVALID_ID { std::numeric_limits< GameObjectID >::max() };

	  private:

		GameObjectID m_id { INVALID_ID };
		GameObjectFlagType object_flags { GameObjectFlagMask::MaskDefault };
		GameObjectTransform m_transform {};

		std::vector< GameObjectComponentPtr > components {};

		std::string m_name {};

		explicit GameObject( GameObjectID obj_id ) : m_id( obj_id ) {}

		FGL_DELETE_DEFAULT_CTOR( GameObject );
		FGL_DELETE_COPY( GameObject );

	  public:

		~GameObject();

		GameObject& operator=( GameObject&& other ) noexcept;
		GameObject( GameObject&& other ) noexcept;

		template < typename T >
			requires is_component< T >
		void addComponent( std::unique_ptr< T >&& ptr )
		{
			components.emplace_back( ptr.release() );
		}

		Scale& getScale() { return m_transform.scale; }

		template < typename T >
			requires is_component< T >
		bool hasComponent() const
		{
			ZoneScoped;
			for ( const GameObjectComponentPtr comp : components )
			{
				if ( comp->id() == T::ID ) return true;
			}

			return false;
		}

		template < typename T >
			requires is_component< T >
		std::vector< const T* > getComponents() const
		{
			ZoneScopedN( "Get components" );
			std::vector< const T* > temp {};

			for ( const ComponentEngineInterface* comp : components )
			{
				if ( comp->id() == T::ID ) temp.emplace_back( static_cast< const T* >( comp ) );
			}

			return temp;
		}

		template < typename T >
			requires is_component< T >
		std::vector< T* > getComponents()
		{
			ZoneScopedN( "Get components" );
			std::vector< T* > temp {};

			for ( ComponentEngineInterface* comp : components )
			{
				if ( comp->id() == T::ID ) temp.emplace_back( static_cast< T* >( comp ) );
			}

			return temp;
		}

		std::vector< GameObjectComponentPtr >& getComponents() { return components; }

		const std::vector< GameObjectComponentPtr >& getComponents() const { return components; }

		//Flags
		GameObjectFlagType flags() const { return object_flags; }

		void addFlag( GameObjectFlagType flag ) { object_flags |= flag; }

		void removeFlag( GameObjectFlagType flag ) { object_flags &= ( ~flag ); }

		//Transform
		GameObjectTransform& getTransform() { return m_transform; }

		const GameObjectTransform& getTransform() const { return m_transform; }

		const WorldCoordinate& getPosition() const { return m_transform.translation; }

		// const Rotation& getRotation() const { return m_transform.rotation; }

		Rotation& getRotation() { return m_transform.rotation; }

		//Misc
		static GameObject createGameObject();

		GameObjectID getId() const { return m_id; }

		//! Returns the name of the game object. If no name is set then the name of the model is used.
		const std::string& getName() const { return m_name; }

		void setName( const std::string& name ) { m_name = name; }

		// void drawImGui();
	};

	//	static_assert( offsetof( GameObject, hot_limter ) < 64, "Hot limit reached" );

} // namespace fgl::engine
