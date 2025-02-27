//
// Created by kj16609 on 2/27/24.
//

#include "TransformComponent.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	glm::mat4 TransformComponent< CType >::mat4() const
	{
		const glm::mat3 rotation_mat { rotation.forcedQuat().mat() };

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

	namespace v2
	{
		TransformComponent::TransformComponent() :
		  m_position( 0.0f, 0.0f, 0.0f ),
		  m_scale( 1.0, 1.0, 1.0 ),
		  m_rotation()
		{}

		Matrix TransformComponent::mat() const
		{
			glm::mat3 rotation_mat { m_rotation.mat() };
			rotation_mat[ 0 ] = rotation_mat[ 0 ] * m_scale.x;
			rotation_mat[ 1 ] = rotation_mat[ 1 ] * m_scale.y;
			rotation_mat[ 2 ] = rotation_mat[ 2 ] * m_scale.z;

			const glm::mat4 mat { { rotation_mat[ 0 ], 0.0f },
				                  { rotation_mat[ 1 ], 0.0f },
				                  { rotation_mat[ 2 ], 0.0f },
				                  { m_position.vec(), 1.0f } };

			const Matrix matrix { mat };

			return matrix;
		}

	} // namespace v2

} // namespace fgl::engine
