//
// Created by kj16609 on 4/5/25.
//
#pragma once
#include "memory/buffers/vector/IndexedVector.hpp"

namespace fgl::engine
{

	struct ModelInstanceInfo
	{
		glm::mat4 m_matrix { glm::mat4( 0.007 ) };
	};

	using ModelInstanceInfoIndex = IndexedVector< ModelInstanceInfo >::Index;

} // namespace fgl::engine
