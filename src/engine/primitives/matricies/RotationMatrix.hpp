//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <glm/mat4x4.hpp>

#include "MatrixEvolvedTypes.hpp"

namespace fgl::engine
{

	class NormalVector;
	class Vector;

	class RotationMatrix : public glm::mat4
	{};

	//This will return a normal vector since it's purely a rotation
	NormalVector operator*( const RotationMatrix rot_mat, const NormalVector vec );
	Vector operator*( const RotationMatrix rot_mat, const Vector vec );

} // namespace fgl::engine
