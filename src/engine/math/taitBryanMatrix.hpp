//
// Created by kj16609 on 2/19/24.
//

#pragma once

#include <glm/mat4x4.hpp>

namespace fgl::engine
{
	struct Rotation;
	class RotationMatrix;

	enum RotationOrder
	{
		XZY,
		XYZ,
		YXZ,
		YZX,
		ZYX,
		ZXY,
		END_OF_ENUM,
		DEFAULT = XZY
	};

	RotationMatrix taitBryanMatrix( const Rotation rotation, const RotationOrder order = DEFAULT );
} // namespace fgl::engine
