//
// Created by kj16609 on 2/28/24.
//

#include <glm/ext/matrix_transform.hpp>

#include "RotationMatrix.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	NormalVector operator*( const RotationMatrix& rot_mat, const NormalVector vec )
	{
		return NormalVector( static_cast< glm::mat3 >( rot_mat ) * vec.vec() );
	}

	Vector operator*( const RotationMatrix& rot_mat, const Vector& vec )
	{
		return Vector( static_cast< glm::mat3 >( rot_mat ) * vec.vec() );
	}

} // namespace fgl::engine
