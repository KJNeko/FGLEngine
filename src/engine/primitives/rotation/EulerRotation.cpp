//
// Created by kj16609 on 2/27/25.
//
#include "EulerRotation.hpp"

namespace fgl::engine
{

	QuatRotation EulerRotation::toRotation() const
	{
		const QuatRotation rotation { x, y, z };

		return rotation;
	}

} // namespace fgl::engine
