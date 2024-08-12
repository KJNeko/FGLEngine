//
// Created by kj16609 on 2/28/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/mat3x3.hpp>
#pragma GCC diagnostic pop

namespace fgl::engine
{

	struct NormalVector;
	class Vector;

	class RotationMatrix : public glm::mat3
	{};

	//This will return a normal vector since it's purely a rotation
	NormalVector operator*( const RotationMatrix& rot_mat, const NormalVector vec );
	Vector operator*( const RotationMatrix& rot_mat, const Vector& vec );

} // namespace fgl::engine
