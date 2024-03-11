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

	class GameObject
	{
	  public:

		using ID = unsigned int;
		using Map = std::unordered_map< ID, GameObject >;

		static constexpr ID INVALID_ID { std::numeric_limits< ID >::max() };

		ID m_id { INVALID_ID };
		bool m_is_static { false };
		bool m_is_visible { true };
		TransformComponent m_transform {};

		//Dummy data to figure out what is above the cache limit
		//bool hot_limter { false };

		std::shared_ptr< Model > m_model { nullptr };

	  private:

		GameObject( ID obj_id ) : m_id( obj_id ) {}

	  public:

		GameObject() {}

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
