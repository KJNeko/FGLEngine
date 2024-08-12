//
// Created by kj16609 on 2/27/24.
//

#include "TransformComponent.hpp"

namespace fgl::engine
{

	glm::mat4 TransformComponent::mat4() const
	{
		const glm::mat3 rotation_mat { rotation.mat() };

		return glm::mat4 { { scale.x * rotation_mat[ 0 ], 0.0f },
			               { scale.y * rotation_mat[ 1 ], 0.0f },
			               { scale.z * rotation_mat[ 2 ], 0.0f },
			               { translation.vec().x, translation.vec().y, translation.vec().z, 1.0f } };
	}

	Matrix< MatrixType::ModelToWorld > TransformComponent::mat() const
	{
		return Matrix< MatrixType::ModelToWorld >( mat4() );
	}

	NormalVector TransformComponent::forward() const
	{
		return rotation.forward();
	}

	NormalVector TransformComponent::right() const
	{
		return rotation.right();
	}

	NormalVector TransformComponent::up() const
	{
		return rotation.up();
	}

} // namespace fgl::engine
