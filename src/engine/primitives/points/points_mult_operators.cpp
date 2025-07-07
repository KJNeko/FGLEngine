//
// Created by kj16609 on 2/28/24.
//

#include "Coordinate.hpp"
#include "engine/primitives/matricies/Matrix.hpp"

namespace fgl::engine
{

	//Specialize
	template <>
	Coordinate< EvolvedType< MatrixType::ModelToWorld >() >
		operator*( const Matrix< MatrixType::ModelToWorld > mat, const Coordinate< CoordinateSpace::Model > coord )
	{
		return Coordinate< EvolvedType< MatrixType::ModelToWorld >() >(
			static_cast< glm::mat4 >( mat ) * glm::vec4( static_cast< glm::vec3 >( coord.vec() ), 1.0f ) );
	}

} // namespace fgl::engine
