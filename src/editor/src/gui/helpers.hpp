//
// Created by kj16609 on 6/5/24.
//

#pragma once

#include "glm/vec3.hpp"
#include "safe_include.hpp"

namespace fgl::engine
{
	struct Rotation;
}

namespace fgl::engine::gui
{
	inline void dragFloat3( const char* const label, glm::vec3& vec )
	{
		ImGui::DragFloat3( label, &vec.x );
	}

	void dragFloat3Rot( const char* label, Rotation& rot );

} // namespace fgl::engine::gui
