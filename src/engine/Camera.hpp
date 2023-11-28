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
		glm::mat4 projection_matrix { 1.f };

	  public:

		const glm::mat4& getProjectionMatrix() const { return projection_matrix; }

		void setOrthographicProjection( float left, float right, float top, float bottom, float near, float far );
		void setPerspectiveProjection( float fovy, float aspect, float near, float far );
	};

} // namespace fgl::engine
