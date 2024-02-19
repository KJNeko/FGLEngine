//
// Created by kj16609 on 2/19/24.
//

#pragma once

#include <glm/mat4x4.hpp>

namespace fgl::engine
{
	class Rotation;

	enum RotationOrder
	{
		XZY,
		XYZ,
		YXZ,
		YZX,
		ZYX,
		ZXY,
		END_OF_ENUM,
		DEFAULT = ZXY
	};

	glm::mat4 taitBryanMatrix( const Rotation rotation, const RotationOrder order = DEFAULT );
} // namespace fgl::engine
