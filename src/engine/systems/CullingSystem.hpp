//
// Created by kj16609 on 2/23/24.
//

#pragma once

#include "engine/FrameInfo.hpp"

namespace fgl::engine
{

	class CullingSystem
	{
	  public:

		void pass( FrameInfo& info );
	};

} // namespace fgl::engine
