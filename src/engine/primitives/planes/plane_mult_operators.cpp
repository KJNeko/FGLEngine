//
// Created by kj16609 on 2/28/24.
//

#include "OriginDistancePlane.hpp"
#include "PointPlane.hpp"
#include "engine/primitives/matricies/Matrix.hpp"

namespace fgl::engine
{

	template <>
	OriginDistancePlane< EvolvedType< MatrixType::ModelToWorld >() > operator*(
		const Matrix< MatrixType::ModelToWorld > matrix, const OriginDistancePlane< CoordinateSpace::Model > plane )
	{
		const NormalVector new_direction { matrix * plane.direction() };
		const Coordinate< EvolvedType< MatrixType::ModelToWorld >() > new_center { matrix * plane.getPosition() };
		const float new_distance { glm::dot( new_center.vec(), new_direction.vec() ) };

		return OriginDistancePlane< EvolvedType< MatrixType::ModelToWorld >() > { new_direction, new_distance };
	}

	template <>
	PointPlane< EvolvedType< MatrixType::ModelToWorld >() >
		operator*( const Matrix< MatrixType::ModelToWorld > mat, const PointPlane< CoordinateSpace::Model > plane )
	{
		return PointPlane<
			EvolvedType< MatrixType::ModelToWorld >() >( mat * plane.getPosition(), mat * plane.getDirection() );
	}

	//	template class PointPlane< CoordinateSpace::Model >;
	template class PointPlane< CoordinateSpace::World >;

	//	template class OriginDistancePlane< CoordinateSpace::Model >;
	template class OriginDistancePlane< CoordinateSpace::World >;

} // namespace fgl::engine
