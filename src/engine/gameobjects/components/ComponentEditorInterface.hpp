//
// Created by kj16609 on 7/7/24.
//

#pragma once

#ifdef TITOR_EDITOR
#include <string_view>
#endif

namespace fgl::engine
{

	struct ComponentEditorInterface
	{
#ifdef TITOR_EDITOR
		virtual void drawImGui() = 0;
		virtual std::string_view name() const = 0;
#endif
		virtual ~ComponentEditorInterface() = default;
	};

} // namespace fgl::engine
