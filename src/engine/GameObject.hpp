//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "Model.hpp"

namespace fgl::engine
{

	struct TransformComponent
	{
		glm::vec3 translation {};
		glm::vec3 scale { 1.0f, 1.0f, 1.0f };
		glm::vec3 rotation {};

		glm::mat4 mat4() const
		{
			const float c3 { glm::cos( rotation.z ) };
			const float s3 { glm::sin( rotation.z ) };
			const float c2 { glm::cos( rotation.x ) };
			const float s2 { glm::sin( rotation.x ) };
			const float c1 { glm::cos( rotation.y ) };
			const float s1 { glm::sin( rotation.y ) };

			return glm::mat4 { { scale.x * ( c1 * c3 + s1 * s2 * s3 ),
				                 scale.x * ( c2 * s3 ),
				                 scale.x * ( c1 * s2 * s3 - c3 * s1 ),
				                 0.0f },
				               { scale.y * ( c3 * s1 * s2 - c1 * s3 ),
				                 scale.y * ( c2 * c3 ),
				                 scale.y * ( c1 * c3 * s2 + s1 * s3 ),
				                 0.0f },
				               { scale.z * ( c2 * s1 ), scale.z * ( -s2 ), scale.z * ( c1 * c2 ), 0.0f },
				               { translation.x, translation.y, translation.z, 1.0f } };
		}
	};

	class GameObject
	{
	  public:

		using ID = unsigned int;

		ID m_id;

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

} // namespace fgl::engine
