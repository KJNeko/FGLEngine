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

	class GameObject
	{
	  public:

		using ID = unsigned int;
		using Map = std::unordered_map< ID, GameObject >;

		static constexpr ID INVALID_ID { std::numeric_limits< ID >::max() };

		ID m_id { INVALID_ID };
		GameObjectFlagType object_flags { GameObjectFlagMask::MASK_DEFAULT };
		TransformComponent m_transform {};

		std::shared_ptr< Model > m_model { nullptr };

	  private:

		GameObject( ID obj_id ) : m_id( obj_id ) {}

		GameObject() = delete;

	  public:

		GameObject( const GameObject& other ) = delete;
		GameObject& operator=( const GameObject& other ) = delete;

		GameObject( GameObject&& other ) = default;
		GameObject& operator=( GameObject&& ) = default;

		inline const WorldCoordinate& getPosition() const { return m_transform.translation; }

		inline const Rotation& getRotation() const { return m_transform.rotation; }

		OrientedBoundingBox< CoordinateSpace::World > getBoundingBox() const;

		static GameObject createGameObject();

		inline ID getId() const { return m_id; }
	};

	//	static_assert( offsetof( GameObject, hot_limter ) < 64, "Hot limit reached" );

} // namespace fgl::engine
