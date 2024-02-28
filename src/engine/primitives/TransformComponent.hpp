//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Rotation.hpp"

namespace fgl::engine
{
	template < MatrixType >
	struct Matrix;

	//TransformComponent is always in world space
	struct TransformComponent
	{
		WorldCoordinate translation { constants::DEFAULT_VEC3 };
		glm::vec3 scale { 1.0f, 1.0f, 1.0f };

		Rotation rotation { 0.0f, 0.0f, 0.0f };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		glm::mat4 mat4() const;

		Matrix< MatrixType::ModelToWorld > mat() const;
	};

} // namespace fgl::engine
