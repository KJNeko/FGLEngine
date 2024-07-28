//
// Created by kj16609 on 7/23/24.
//

#pragma once

namespace fgl::engine
{
	struct CameraInfo
	{
		glm::mat4 projection { 1.0f };
		glm::mat4 view { 1.0f };
		glm::mat4 inverse_view { 1.0f };
	};


} // namespace fgl::engine