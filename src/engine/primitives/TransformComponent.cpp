//
// Created by kj16609 on 2/27/24.
//

#include "TransformComponent.hpp"

#include "engine/math/taitBryanMatrix.hpp"

namespace fgl::engine
{

	glm::mat4 TransformComponent::mat4() const
	{
		const glm::mat3 rotation_mat { rotation.mat() };

		//TODO: This uses the Tait-Bryan angles stuff, It should use the respective function.
		// Or maybe something else can be done here
		return glm::mat4 { { scale.x * rotation_mat[ 0 ][ 0 ],
			                 scale.x * rotation_mat[ 0 ][ 1 ],
			                 scale.x * rotation_mat[ 0 ][ 2 ],
			                 0.0f },
			               { scale.y * rotation_mat[ 1 ][ 0 ],
			                 scale.y * rotation_mat[ 1 ][ 1 ],
			                 scale.y * rotation_mat[ 1 ][ 2 ],
			                 0.0f },
			               { scale.z * rotation_mat[ 2 ][ 0 ],
			                 scale.z * rotation_mat[ 2 ][ 1 ],
			                 scale.z * rotation_mat[ 2 ][ 2 ],
			                 0.0f },
			               { translation.x, translation.y, translation.z, 1.0f } };
	}

	Matrix< MatrixType::ModelToWorld > TransformComponent::mat() const
	{
		return Matrix< MatrixType::ModelToWorld >( mat4() );
	}

} // namespace fgl::engine
