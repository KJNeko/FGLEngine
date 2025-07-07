//
// Created by kj16609 on 2/17/25.
//
#pragma once

#include "primitives/vectors/NormalVector.hpp"

namespace fgl::engine::lights
{

	//! Sun illumation.
	class GlobalIlluminator
	{
		NormalVector m_direction;

	  public:

		explicit GlobalIlluminator( NormalVector direction );
	};

} // namespace fgl::engine::lights

namespace fgl::engine
{
	using namespace lights;
}