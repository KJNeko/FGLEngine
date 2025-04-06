//
// Created by kj16609 on 4/5/25.
//
#pragma once
#include "memory/buffers/vector/IndexedVector.hpp"

namespace fgl::engine
{

	struct ModelInstanceInfo
	{
		glm::mat4 m_matrix { constants::MAT4_IDENTITY };
	};

	using ModelInstanceInfoIndex = IndexedVector< ModelInstanceInfo >::Index;

} // namespace fgl::engine
