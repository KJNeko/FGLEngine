//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Rotation.hpp"
#include "Vector.hpp"
#include "engine/primitives/Matrix.hpp"

namespace fgl::engine
{

	//TransformComponent is always in world space
	struct TransformComponent
	{
		WorldCoordinate translation { constants::DEFAULT_VEC3 };
		glm::vec3 scale { 1.0f, 1.0f, 1.0f };

		Rotation rotation { 0.0f, 0.0f, 0.0f };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		glm::mat4 mat4() const;

		inline Matrix< MatrixType::ModelToWorld > mat() const { return Matrix< MatrixType::ModelToWorld >( mat4() ); }
	};

} // namespace fgl::engine
