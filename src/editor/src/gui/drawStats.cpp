
#include "core.hpp"
#include "safe_include.hpp"

namespace fgl::engine::gui
{

	void drawStats( const FrameInfo& info )
	{
		ImGui::Begin( "Stats" );

		ImGui::Text( "FPS: %0.1f", ImGui::GetIO().Framerate );

		ImGui::End();
	}

} // namespace fgl::engine::gui
