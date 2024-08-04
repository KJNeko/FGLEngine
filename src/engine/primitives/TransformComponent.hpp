//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Rotation.hpp"
#include "Scale.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	template < MatrixType >
	class Matrix;

	//TransformComponent is always in world space
	struct TransformComponent
	{
		WorldCoordinate translation { constants::WORLD_CENTER };
		Scale scale { 1.0f, 1.0f, 1.0f };

		Rotation rotation { 0.0f, 0.0f, 0.0f };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		glm::mat4 mat4() const;

		Matrix< MatrixType::ModelToWorld > mat() const;

		NormalVector forward() const;

		NormalVector backwards() const { return -forward(); }

		NormalVector right() const;

		NormalVector left() const { return -right(); }

		NormalVector up() const;

		NormalVector down() const { return -up(); }
	};

} // namespace fgl::engine
