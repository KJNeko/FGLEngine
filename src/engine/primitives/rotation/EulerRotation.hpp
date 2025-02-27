//
// Created by kj16609 on 2/27/25.
//
#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "glm/vec3.hpp"
#pragma GCC diagnostic pop

#include "QuatRotation.hpp"

namespace fgl::engine
{

	struct EulerRotation : private glm::vec3
	{
	  public:

		using glm::vec3::x;
		using glm::vec3::y;
		using glm::vec3::z;

		glm::vec3& vec() { return *this; }

		QuatRotation toRotation() const;

		EulerRotation( const glm::vec3& rotation ) : glm::vec3( rotation ) {}
	};

} // namespace fgl::engine