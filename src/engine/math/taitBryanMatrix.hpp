//
// Created by kj16609 on 2/19/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/mat4x4.hpp>
#pragma GCC diagnostic pop

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

	glm::mat3 taitBryanMatrix( const glm::vec3 rotation, const RotationOrder order = DEFAULT );

	inline glm::mat3 taitBryanMatrix( const float x, const float y, const float z, const RotationOrder order = DEFAULT )
	{
		return taitBryanMatrix( glm::vec3( x, y, z ), order );
	}

} // namespace fgl::engine
