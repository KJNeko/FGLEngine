//
// Created by kj16609 on 2/27/24.
//

#include "TransformComponent.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	glm::mat4 TransformComponent< CType >::mat4() const
	{
		const glm::mat3 rotation_mat { rotation.mat() };

		// We must flip the z axis in order to match vulkan. Where 0,0 is the top left of the screen and Z+ is down

		return glm::mat4 { { scale.x * rotation_mat[ 0 ], 0.0f },
			               { scale.y * rotation_mat[ 1 ], 0.0f },
			               { scale.z * rotation_mat[ 2 ], 0.0f },
			               { translation.vec().x, translation.vec().y, translation.vec().z, 1.0f } };
	}

	template < CoordinateSpace CType >
	Matrix< MatrixTransformType< CType >() > TransformComponent< CType >::mat() const
	{
		return Matrix< MatrixTransformType< CType >() >( mat4() );
	}

	template < CoordinateSpace CType >
	NormalVector TransformComponent< CType >::forward() const
	{
		return rotation.forward();
	}

	template < CoordinateSpace CType >
	NormalVector TransformComponent< CType >::right() const
	{
		return rotation.right();
	}

	template < CoordinateSpace CType >
	NormalVector TransformComponent< CType >::up() const
	{
		return rotation.up();
	}

	template struct TransformComponent< CoordinateSpace::World >;
	template struct TransformComponent< CoordinateSpace::Model >;

} // namespace fgl::engine
