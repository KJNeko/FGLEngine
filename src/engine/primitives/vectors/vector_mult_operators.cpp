//
// Created by kj16609 on 2/28/24.
//

#include "NormalVector.hpp"
#include "Vector.hpp"
#include "engine/primitives/matricies/Matrix.hpp"

namespace fgl::engine
{

	template <>
	Vector operator*( const Matrix< MatrixType::ModelToWorld > mat, const Vector vector )
	{
		return Vector( static_cast< glm::mat4 >( mat ) * glm::vec4( vector.vec(), 0.0f ) );
	}

	template <>
	NormalVector operator*( const Matrix< MatrixType::ModelToWorld > mat, const NormalVector vector )
	{
		return NormalVector( static_cast< glm::mat4 >( mat ) * glm::vec4( vector.vec(), 0.0f ) );
	}
} // namespace fgl::engine
