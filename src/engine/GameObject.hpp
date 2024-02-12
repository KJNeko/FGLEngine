//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

#include "constants.hpp"

namespace fgl::engine
{

	class Model;

	struct TransformComponent
	{
		glm::vec3 translation { constants::DEFAULT_VEC3 };
		glm::vec3 scale { 1.0f, 1.0f, 1.0f };
		glm::vec3 rotation { 0.0f, 0.0f, 0.0f };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		glm::mat4 mat4() const;

		glm::mat3 normalMatrix() const;
	};

	struct PointLightComponent
	{
		float light_intensity { 1.0f };
	};

	class GameObject
	{
	  public:

		using ID = unsigned int;
		using Map = std::unordered_map< ID, GameObject >;

		ID m_id;
		bool is_world { false };

		std::shared_ptr< Model > model {};
		glm::vec3 color {};
		TransformComponent transform {};
		std::unique_ptr< PointLightComponent > point_light { nullptr };

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

		static GameObject
			makePointLight( float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3( 1.0f ) );

		ID getId() const { return m_id; }
	};

} // namespace fgl::engine
