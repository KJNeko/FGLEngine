//
// Created by kj16609 on 7/7/24.
//

#pragma once

#include <format>
#include <string_view>

namespace fgl::engine
{

	struct ComponentEditorInterface
	{
		virtual void drawImGui() = 0;

		[[nodiscard]] virtual std::string_view humanName() const = 0;
		[[nodiscard]] virtual std::string_view className() const = 0;

		[[nodiscard]] std::string name() const { return std::format( "{} ({})", humanName(), className() ); }

		virtual ~ComponentEditorInterface() = default;
	};

} // namespace fgl::engine
