//
// Created by kj16609 on 4/16/25.
//
#pragma once
#include "rendering/types.hpp"

namespace fgl::engine::global
{

	void setCurrentFrameIndex( FrameIndex index );

	FrameIndex getCurrentFrameIndex();

} // namespace fgl::engine::global
