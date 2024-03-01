//
// Created by kj16609 on 2/28/24.
//

#include "RotationMatrix.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	NormalVector operator*( const RotationMatrix rot_mat, const NormalVector vec )
	{
		return NormalVector( static_cast< glm::mat4 >( rot_mat ) * glm::vec4( vec.vec(), 0.0f ) );
	}

	Vector operator*( const RotationMatrix rot_mat, const Vector vec )
	{
		return Vector( static_cast< glm::mat4 >( rot_mat ) * glm::vec4( vec.vec(), 0.0f ) );
	}

} // namespace fgl::engine
