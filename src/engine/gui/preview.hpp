//
// Created by kj16609 on 6/24/24.
//

#pragma once

namespace fgl::engine
{
	struct FrameInfo;
	class Camera;
} // namespace fgl::engine

namespace fgl::engine::gui
{

	void drawRenderingOutputs( FrameInfo& info, const Camera& camera );

} // namespace fgl::engine::gui
