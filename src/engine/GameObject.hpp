//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>
#include <unordered_map>

#include "engine/primitives/TransformComponent.hpp"

namespace fgl::engine
{

	class Model;

	class GameObject
	{
	  public:

		using ID = unsigned int;
		using Map = std::unordered_map< ID, GameObject >;

		//Hot
		ID m_id;
		bool is_world { false };
		bool is_visible { true };

		//Dummy data to figure out what is above the cache limit
		bool hot_limter { false };

		//Cold
		std::shared_ptr< Model > model {};
		glm::vec3 color {};
		TransformComponent transform {};

	  private:

		GameObject( ID obj_id ) : m_id( obj_id ) {}

	  public:

		GameObject() = delete;
		GameObject( const GameObject& other ) = delete;
		GameObject& operator=( const GameObject& other ) = delete;
		GameObject( GameObject&& other ) = default;
		GameObject& operator=( GameObject&& ) = default;

		static GameObject createGameObject()
		{
			static ID current_id { 0 };
			return { current_id++ };
		}

		ID getId() const { return m_id; }
	};

	static_assert( offsetof( GameObject, hot_limter ) < 64, "Hot limit reached" );

} // namespace fgl::engine
