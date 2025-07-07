//
// Created by kj16609 on 4/5/25.
//
#pragma once
#include "memory/buffers/vector/IndexedVector.hpp"

namespace fgl::engine
{

	struct ModelInstanceInfo
	{
		glm::mat4 m_model_matrix { glm::mat4( constants::DEFAULT_MODEL_SCALE ) };
		// glm::mat4 m_normal_matrix { glm::transpose( glm::inverse( m_model_matrix ) ) };
	};

	using ModelInstanceInfoIndex = IndexedVector< ModelInstanceInfo >::Index;

} // namespace fgl::engine
