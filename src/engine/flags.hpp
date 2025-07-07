//
// Created by kj16609 on 10/18/24.
//

// This file is supposed to contain various global flags used by the program

#pragma once

namespace fgl::engine::flags
{
	void triggerShaderReload();

	bool shouldReloadShaders();

	//! Resets any flags that can be reset after the frame is done
	void resetFlags();

} // namespace fgl::engine::flags