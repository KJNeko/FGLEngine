//
// Created by kj16609 on 6/5/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "glm/vec3.hpp"
#pragma GCC diagnostic pop
#include "safe_include.hpp"

namespace fgl::engine
{
	struct EulerRotation;
	struct QuatRotation;
} // namespace fgl::engine

namespace fgl::engine::gui
{
	inline void dragFloat3( const char* const label, glm::vec3& vec )
	{
		ImGui::DragFloat3( label, &vec.x );
	}

	void dragFloat3Rot( const char* label, EulerRotation& rot );

} // namespace fgl::engine::gui
