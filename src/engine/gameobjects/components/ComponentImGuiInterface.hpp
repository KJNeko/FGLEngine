//
// Created by kj16609 on 7/7/24.
//

#pragma once

namespace fgl::engine
{

	struct ComponentImGuiInterface
	{
		virtual void drawImGui() = 0;
		virtual ~ComponentImGuiInterface() = default;
	};


}

