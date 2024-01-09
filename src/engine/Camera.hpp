//
// Created by kj16609 on 11/28/23.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace fgl::engine
{
	class Camera
	{
		glm::mat4 projection_matrix { 1.0f };
		glm::mat4 view_matrix { 1.0f };
		glm::mat4 inverse_view_matrix { 1.0f };

	  public:

		const glm::mat4& getProjectionMatrix() const { return projection_matrix; }

		const glm::mat4& getViewMatrix() const { return view_matrix; }

		const glm::mat4& getInverseView() const { return inverse_view_matrix; }

		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );

		const glm::vec3 getPosition() const { return glm::vec3( inverse_view_matrix[ 3 ] ); }

		void setViewDirection( glm::vec3 pos, glm::vec3 direction, glm::vec3 up = glm::vec3 { 0.0f, -1.0f, 0.0f } );
		void setViewTarget( glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3 { 0.0f, -1.0f, 0.0f } );
		void setViewYXZ( glm::vec3 pos, glm::vec3 rotation );
	};

} // namespace fgl::engine
