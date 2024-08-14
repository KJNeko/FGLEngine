//
// Created by kj16609 on 7/7/24.
//

#pragma once

#include <string_view>

namespace fgl::engine
{

	struct ComponentEditorInterface
	{
		virtual void drawImGui() = 0;
		virtual std::string_view name() const = 0;
		virtual ~ComponentEditorInterface() = default;
	};

} // namespace fgl::engine
